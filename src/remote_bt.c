#include "remote_bt.h"
#include "bencode.c"
#include "ssh_config.c"

#define REMOTE_BT_BUFFER_SIZE 1024 * 4096

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "no torrent link specified\n");
		return 1;
	}

	ssh_session remote_session = start_ssh_connection();
	if (remote_session == NULL)
	{
		fprintf(stderr, "failed to connect to ssh remote\n");
		return 1;
	}

	char *link = argv[1];
	size_t link_size = strlen(link);
	char *curl_command_start = "curl --output - '";
	size_t curl_command_start_size = strlen(curl_command_start);
	char *curl_command_end = "'";
	size_t curl_command_end_size = strlen(curl_command_end);
	size_t curl_command_size = curl_command_start_size + link_size + curl_command_end_size;
	char *curl_command = (char *)calloc(curl_command_size + 1, sizeof(char));
	memcpy(curl_command, curl_command_start, curl_command_start_size);
	memcpy(curl_command + curl_command_start_size, link, link_size);
	memcpy(curl_command + curl_command_start_size + link_size, curl_command_end, curl_command_end_size);

	u8_array torrent;
	if (run_remote_command(remote_session, curl_command, &torrent) != 0)
	{
		fprintf(stderr, "failed to run remote command\n");
		free(curl_command);
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return 1;
	}

	free(curl_command);
	ssh_disconnect(remote_session);
	ssh_free(remote_session);

	u8_array bencoded_announce;
	if (bencode_get_value_for_key(torrent, "announce", 8, &bencoded_announce) != 0)
	{
		fprintf(stderr, "did not find announce key in dictionary\n");
		free(torrent.data);
		return 1;
	}

	char *announce = bencode_allocate_string_value(bencoded_announce);
	if (announce == NULL)
	{
		fprintf(stderr, "failed to convert bencoded value into a string\n");
		free(torrent.data);
		return 1;
	}

	fprintf(stdout, "%s\n", announce);
	free(announce);
	free(torrent.data);
	return 0;
}

ssh_session start_ssh_connection()
{
	ssh_session remote_session = ssh_new();
	if (remote_session == NULL)
	{
		fprintf(stderr, "failed to initialize ssh\n");
		return NULL;
	}

	if (ssh_options_set(remote_session, SSH_OPTIONS_HOST, ssh_host) != 0)
	{
		fprintf(stderr, "failed to set ssh host option\n");
		ssh_free(remote_session);
		return NULL;
	}

	if (ssh_options_set(remote_session, SSH_OPTIONS_PORT, &ssh_port) != 0)
	{
		fprintf(stderr, "failed to set ssh port option\n");
		ssh_free(remote_session);
		return NULL;
	}

	if (ssh_options_set(remote_session, SSH_OPTIONS_USER, ssh_user) != 0)
	{
		fprintf(stderr, "failed to set ssh user option\n");
		ssh_free(remote_session);
		return NULL;
	}

	if (ssh_connect(remote_session) != SSH_OK)
	{
		fprintf(stderr, "failed to connect to remote: %s\n", ssh_get_error(remote_session));
		ssh_free(remote_session);
		return NULL;
	}

	// TODO: check known_hosts, generate keypair if needed, write to known_hosts, etc

	ssh_key public_key;
	if (ssh_pki_import_pubkey_file(ssh_public_key_file, &public_key) != SSH_OK)
	{
		fprintf(stderr, "failed to read public key %s: %s\n", ssh_public_key_file, ssh_get_error(remote_session));
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return NULL;
	}

	if (ssh_userauth_try_publickey(remote_session, NULL, public_key) != SSH_AUTH_SUCCESS)
	{
		fprintf(stderr, "public key %s not authorized: %s\n", ssh_public_key_file, ssh_get_error(remote_session));
		ssh_key_free(public_key);
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return NULL;
	}	

	ssh_key private_key;
	if (ssh_pki_import_privkey_file(ssh_private_key_file, ssh_private_key_password, NULL, NULL, &private_key) != SSH_OK)
	{
		fprintf(stderr, "failed to read private key %s: %s\n", ssh_private_key_file, ssh_get_error(remote_session));
		ssh_key_free(public_key);
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return NULL;
	}

	if (ssh_userauth_publickey(remote_session, NULL, private_key) != SSH_AUTH_SUCCESS)
	{
		fprintf(stderr, "private key %s not authorized: %s\n", ssh_private_key_file, ssh_get_error(remote_session));
		ssh_key_free(private_key);
		ssh_key_free(public_key);
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return NULL;
	}

	ssh_key_free(private_key);
	ssh_key_free(public_key);
	return remote_session;
}

int run_remote_command(ssh_session remote_session, char *command, u8_array *out_stdout_data)
{
	ssh_channel channel = ssh_channel_new(remote_session);
	if (channel == NULL)
	{
		fprintf(stderr, "failed to create ssh channel: %s\n", ssh_get_error(remote_session));
		return 1;
	}

	if (ssh_channel_open_session(channel) != SSH_OK)
	{
		fprintf(stderr, "failed to start ssh channel: %s\n", ssh_get_error(remote_session));
		ssh_channel_free(channel);
		return 1;
	}

	if (ssh_channel_request_exec(channel, command) != SSH_OK)
	{
		fprintf(stderr, "execute remote command %s: %s\n", command, ssh_get_error(remote_session));
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return 1;
	}

	u8 *buffer = (u8 *)calloc(sizeof(u8), REMOTE_BT_BUFFER_SIZE);
	if (buffer == NULL)
	{
		fprintf(stderr, "failed to allocate buffer memory for reading remote command output\n");
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return 1;
	}

	out_stdout_data->size = 0;
	size_t num_bytes_read = ssh_channel_read(channel, buffer, REMOTE_BT_BUFFER_SIZE, 0);
	while (num_bytes_read > 0)
	{
		if (out_stdout_data->size == 0)
		{
			out_stdout_data->size = num_bytes_read;
			out_stdout_data->data = (u8 *)calloc(sizeof(u8), num_bytes_read);
			if (out_stdout_data->data == NULL)
			{
				fprintf(stderr, "failed to allocate memory for storing remote command output\n");
				free(buffer);
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				return 1;
			}

			memcpy(out_stdout_data->data, buffer, num_bytes_read);
		}
		else
		{
			size_t expanded_size = out_stdout_data->size + num_bytes_read;
			u8 *expanded_data = (u8 *)calloc(sizeof(u8), expanded_size);
			if (expanded_data == NULL)
			{
				fprintf(stderr, "failed to allocate memory for storing remote command output\n");
				free(out_stdout_data->data);
				free(buffer);
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				return 1;
			}

			memcpy(expanded_data, out_stdout_data->data, out_stdout_data->size);
			memcpy(expanded_data + out_stdout_data->size, buffer, num_bytes_read);
			free(out_stdout_data->data);
			out_stdout_data->size = expanded_size;
			out_stdout_data->data = expanded_data;
		}

		num_bytes_read = ssh_channel_read(channel, buffer, REMOTE_BT_BUFFER_SIZE, 0);
	}

	if (num_bytes_read < 0)
	{
		fprintf(stderr, "error while reading data from remote\n");
		free(out_stdout_data->data);
		free(buffer);
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return 1;
	}

	free(buffer);
	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);
	return 0;
}

