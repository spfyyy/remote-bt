#ifndef TORRENT_H
#define TORRENT_H

#include <stdint.h>

#define INFO_HASH_SIZE 20

typedef struct torrent_metadata
{
	char *announce;
	char *name;
	int64_t piece_length;
	char *pieces;
	int64_t length; // TODO: support multifile
	int32_t is_multifile;
	uint8_t *info_hash;
} torrent_metadata;

torrent_metadata *torrent_allocate_metadata_from_dictionary(uint8_t *dict, size_t dict_size);

#endif
