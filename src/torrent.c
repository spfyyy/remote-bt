#include "libssh/libssh.h"
#include "bencode.c"
#include "ssh_config.c"

i32 countNumBytesInFile(FILE *file, size_t *outCount);
i32 allocMemoryAndLoadFile(char *fileName, u8 **outMemory, size_t *outMemorySize);

int main(int argc, char **argv)
{
	ssh_session remote_session = ssh_new();
	if (remote_session == NULL)
	{
		fprintf(stderr, "failed to initialize ssh\n");
		return 1;
	}

	ssh_options_set(remote_session, SSH_OPTIONS_HOST, host);
	ssh_options_set(remote_session, SSH_OPTIONS_PORT, &port);
	ssh_options_set(remote_session, SSH_OPTIONS_USER, user);

	if (ssh_connect(remote_session) != SSH_OK)
	{
		fprintf(stderr, "failed to connect to remote: %s\n", ssh_get_error(remote_session));
		ssh_free(remote_session);
		return 1;
	}

	// TODO: check known_hosts, generate keypair if needed, write to known_hosts, etc

	ssh_key key;
	if (ssh_pki_import_pubkey_file(public_key, &key) != SSH_OK)
	{
		fprintf(stderr, "failed to read public key: %s\n", public_key);
		ssh_disconnect(remote_session);
		ssh_free(remote_session);
		return 1;
	}

	ssh_key_free(key);
	ssh_disconnect(remote_session);
	ssh_free(remote_session);

	if (argc < 2)
	{
		fprintf(stderr, "no .torrent file specified\n");
		return 1;
	}

	char *fileName = argv[1];
	u8 *fileBuffer;
	size_t fileSize;
	if (allocMemoryAndLoadFile(fileName, &fileBuffer, &fileSize) != 0)
	{
		fprintf(stderr, "failed to load file\n");
		return 1;
	}

	u8_array torrent;
	torrent.data = fileBuffer;
	torrent.size = fileSize;

	u8_array bencoded_announce;
	if (bencode_get_value_for_key(torrent, "announce", 8, &bencoded_announce) != 0)
	{
		fprintf(stderr, "did not find announce key in dictionary\n");
		return 1;
	}

	char *announce = bencode_allocate_string_value(bencoded_announce);
	if (announce == NULL)
	{
		fprintf(stderr, "failed to convert bencoded value into a string\n");
		return 1;
	}

	fprintf(stdout, "%s\n", announce);
	free(announce);
	return 0;
}

i32 countNumBytesInFile(FILE *file, size_t *outCount)
{
	size_t bufferSize = 1024 * 4096;
	u8 *buffer = (u8 *)calloc(sizeof(u8), bufferSize);
	if (buffer == NULL)
	{
		return 1;
	}

	*outCount = fread(buffer, sizeof(u8), bufferSize, file);
	while (!feof(file) && !ferror(file))
	{
		*outCount += fread(buffer, sizeof(u8), bufferSize, file);
	}
	free(buffer);

	i32 error = ferror(file);
	rewind(file);
	return error;
}

i32 allocMemoryAndLoadFile(char *fileName, u8 **outMemory, size_t *outMemorySize)
{
	FILE *file = fopen(fileName, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "could not open torrent file\n");
		return 1;
	}

	if (countNumBytesInFile(file, outMemorySize) != 0)
	{
		fprintf(stderr, "failed to get file size for torrent file\n");
		fclose(file);
		return 1;
	}

	*outMemory = (u8 *)calloc(sizeof(u8), *outMemorySize);
	if (*outMemory == NULL)
	{
		fprintf(stderr, "failed to allocate memory for storing torrent file\n");
		fclose(file);
		return 1;
	}

	size_t bytesRead = fread(*outMemory, sizeof(u8), *outMemorySize, file);
	if (bytesRead != *outMemorySize)
	{
		fprintf(stderr, "unexpected number of bytes read from torrent file\n");
		fclose(file);
		free(*outMemory);
		return 1;
	}

	fclose(file);
	return 0;
}
