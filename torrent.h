#ifndef TORRENT_H
#define TORRENT_H

#include <stdint.h>
#include "bencode.h"

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

torrent_metadata *torrent_allocate_metadata_from_dictionary(bencode_dictionary in_dictionary);

void torrent_free_metadata(torrent_metadata *metadata);

#endif
