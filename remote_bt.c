#include <libssh/libssh.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "remote_bt.h"
#include "types.h"
#include "torrent.h"
#include "ssh_config.c"

#ifndef REMOTE_BT_COMMAND_MAX_LENGTH
#define REMOTE_BT_COMMAND_MAX_LENGTH 300
#endif
#ifndef REMOTE_BT_BUFFER_SIZE
#define REMOTE_BT_BUFFER_SIZE 1024 * 4096
#endif

// utility methods
static ssh_session start_ssh_connection(void);
static void end_ssh_connection(ssh_session in_remote_session);
static int run_remote_command(ssh_session in_remote_session, char *in_command, u8_array *out_stdout_data);
static int fetch_torrent_file(ssh_session in_remote_session, char *in_link, u8_array *out_torrent);
static char *allocate_command(char *in_format, ...);

int remote_bt_init(void)
{
	// initialize downloads/threads
	return 0;
}

void remote_bt_shutdown(void)
{
	// shutdown downloads/threads
}

int remote_bt_download(char *link)
{
	if (link == NULL)
	{
		fprintf(stderr, "torrent link is null\n");
		return 1;
	}

	ssh_session remote_session = start_ssh_connection();
	if (remote_session == NULL)
	{
		fprintf(stderr, "failed to start remote session\n");
		return 1;
	}

	u8_array torrent_raw;
	if (fetch_torrent_file(remote_session, link, &torrent_raw) != 0)
	{
		fprintf(stderr, "failed to run remote command\n");
		end_ssh_connection(remote_session);
		return 1;
	}

	bencode_dictionary *dict = bencode_allocate_dictionary((bencode_data){torrent_raw.data,torrent_raw.size});
	free(torrent_raw.data);
	torrent_metadata *t = torrent_allocate_metadata_from_dictionary(*dict);
	bencode_free_dictionary(dict);

	fprintf(stdout, "name: %s\n", t->name);
	fprintf(stdout, "announce: %s\n", t->announce);
	fprintf(stdout, "piece length: %lld bytes\n", t->piece_length);
	fprintf(stdout, "is_multifile: %s\n", t->is_multifile ? "true" : "false");
	torrent_free_metadata(t);

	end_ssh_connection(remote_session);
	return 0;
}

static ssh_session start_ssh_connection(void)
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

static void end_ssh_connection(ssh_session in_remote_session)
{
	ssh_disconnect(in_remote_session);
	ssh_free(in_remote_session);
}

static int run_remote_command(ssh_session in_remote_session, char *in_command, u8_array *out_stdout_data)
{
	ssh_channel channel = ssh_channel_new(in_remote_session);
	if (channel == NULL)
	{
		fprintf(stderr, "failed to create ssh channel: %s\n", ssh_get_error(in_remote_session));
		return 1;
	}

	if (ssh_channel_open_session(channel) != SSH_OK)
	{
		fprintf(stderr, "failed to start ssh channel: %s\n", ssh_get_error(in_remote_session));
		ssh_channel_free(channel);
		return 1;
	}

	if (ssh_channel_request_exec(channel, in_command) != SSH_OK)
	{
		fprintf(stderr, "execute remote command %s: %s\n", in_command, ssh_get_error(in_remote_session));
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return 1;
	}

	uint8_t *buffer = (uint8_t *)calloc(REMOTE_BT_BUFFER_SIZE, sizeof(uint8_t));
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
			out_stdout_data->data = (uint8_t *)calloc(num_bytes_read, sizeof(uint8_t));
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
			uint8_t *expanded_data = (uint8_t *)calloc(expanded_size, sizeof(uint8_t));
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

static int fetch_torrent_file(ssh_session in_remote_session, char *in_link, u8_array *out_torrent)
{
	char *curl_command = allocate_command("curl --output - '%s'", in_link);
	if (curl_command == NULL)
	{
		fprintf(stderr, "failed to allocate curl command\n");
		return 1;
	}

	int result_code = run_remote_command(in_remote_session, curl_command, out_torrent);
	free(curl_command);
	return result_code;
}

static char *allocate_command(char *in_format, ...)
{
	va_list arglist;
	va_start(arglist, in_format);
	char *command = (char *)calloc(REMOTE_BT_COMMAND_MAX_LENGTH, sizeof(char));
	if (command == NULL)
	{
		fprintf(stderr, "failed to allocate memory for command of format: %s\n", in_format);
		va_end(arglist);
		return NULL;
	}

	int command_length = vsnprintf(command, REMOTE_BT_COMMAND_MAX_LENGTH, in_format, arglist);
	if (command_length < 0 || command_length >= REMOTE_BT_COMMAND_MAX_LENGTH)
	{
		fprintf(stderr, "failed to format command with format: %s\n", in_format);
		free(command);
		va_end(arglist);
		return NULL;
	}

	va_end(arglist);
	return command;
}
