#include "remote_bt.h"
#include "bencode.c"
#include "ssh_config.c"

#define COMMAND_MAX_LENGTH 300
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
	char *curl_command = (char *)calloc(COMMAND_MAX_LENGTH, sizeof(char));
	int curl_command_length = snprintf(curl_command, COMMAND_MAX_LENGTH, "curl --output - '%s'", link);
	if (curl_command_length < 0 || curl_command_length >= COMMAND_MAX_LENGTH)
	{
		fprintf(stderr, "failed to allocate curl command\n");
		return 1;
	}

	i32 result_code;
	u8_array torrent;
	result_code = run_remote_command(remote_session, curl_command, &torrent);
	free(curl_command);
	ssh_disconnect(remote_session);
	ssh_free(remote_session);
	if (result_code != 0)
	{
		fprintf(stderr, "failed to run remote command\n");
		return 1;
	}

	u8_array bencoded_announce;
	result_code = bencode_get_value_for_key(torrent, "announce", 8, &bencoded_announce);
	if (result_code != 0)
	{
		fprintf(stderr, "did not find announce key in dictionary\n");
		free(torrent.data);
		return 1;
	}

	char *announce = bencode_allocate_string_value(bencoded_announce);
	free(torrent.data);
	if (announce == NULL)
	{
		fprintf(stderr, "failed to convert bencoded value into a string\n");
		return 1;
	}

	fprintf(stdout, "%s\n", announce);
	free(announce);
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

	u8 *buffer = (u8 *)calloc(REMOTE_BT_BUFFER_SIZE, sizeof(u8));
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
			out_stdout_data->data = (u8 *)calloc(num_bytes_read, sizeof(u8));
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
			u8 *expanded_data = (u8 *)calloc(expanded_size, sizeof(u8));
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
