#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "bencode.h"
#include "torrent.h"

torrent_metadata *torrent_allocate_metadata_from_dictionary(uint8_t *dict, size_t dict_size)
{
	uint8_t *bencoded_announce;
	size_t bencoded_announce_size;
	if (bencode_value_for_key(dict, dict_size, "announce", 8, &bencoded_announce, &bencoded_announce_size) != 0)
	{
		return NULL;
	}

	uint8_t *announce;
	size_t announce_length;
	if (bencode_parse_string(bencoded_announce, bencoded_announce_size, &announce, &announce_length) != 0)
	{
		return NULL;
	}

	uint8_t *info;
	size_t info_size;
	if (bencode_value_for_key(dict, dict_size, "info", 4, &info, &info_size) != 0)
	{
		return NULL;
	}

	uint8_t *bencoded_name;
	size_t bencoded_name_size;
	if (bencode_value_for_key(info, info_size, "name", 4, &bencoded_name, &bencoded_name_size) != 0)
	{
		return NULL;
	}

	uint8_t *name;
	size_t name_length;
	if (bencode_parse_string(bencoded_name, bencoded_name_size, &name, &name_length) != 0)
	{
		return NULL;
	}

	uint8_t *bencoded_piece_length;
	size_t bencoded_piece_length_size;
	if (bencode_value_for_key(info, info_size, "piece length", 12, &bencoded_piece_length, &bencoded_piece_length_size) != 0)
	{
		return NULL;
	}

	int64_t piece_length;
	if (bencode_parse_integer(bencoded_piece_length, bencoded_piece_length_size, &piece_length) != 0)
	{
		return NULL;
	}

	uint8_t *bencoded_pieces;
	size_t bencoded_pieces_size;
	if (bencode_value_for_key(info, info_size, "pieces", 6, &bencoded_pieces, &bencoded_pieces_size) != 0)
	{
		return NULL;
	}

	uint8_t *pieces;
	size_t pieces_length;
	if (bencode_parse_string(bencoded_pieces, bencoded_pieces_size, &pieces, &pieces_length) != 0)
	{
		return NULL;
	}

	uint8_t *bencoded_length;
	size_t bencoded_length_size;
	if (bencode_value_for_key(info, info_size, "length", 6, &bencoded_length, &bencoded_length_size) != 0)
	{
		return NULL;
	}

	int64_t length;
	if (bencode_parse_integer(bencoded_length, bencoded_length_size, &length) != 0)
	{
		return NULL;
	}

	size_t metadata_size = sizeof(torrent_metadata);
	size_t announce_size = announce_length + 1;
	size_t name_size = name_length + 1;
	size_t pieces_size = pieces_length + 1;
	size_t allocated_size = metadata_size + announce_size + name_size + pieces_size + INFO_HASH_SIZE;
	uint8_t *data = (uint8_t *)calloc(allocated_size, sizeof(uint8_t));
	if (data == NULL)
	{
		return NULL;
	}

	torrent_metadata *metadata = (torrent_metadata *)data;
	metadata->announce = (char *)(data + metadata_size);
	metadata->name = (char *)(data + metadata_size + announce_size);
	metadata->pieces = (char *)(data + metadata_size + announce_size + name_size);
	metadata->info_hash = (uint8_t *)(data + metadata_size + announce_size + name_size + pieces_size);
	memcpy(metadata->announce, announce, announce_length);
	memcpy(metadata->name, name, name_length);
	memcpy(metadata->pieces, pieces, pieces_length);
	SHA1(info, info_size, metadata->info_hash);
	metadata->piece_length = piece_length;
	metadata->length = length;
	metadata->is_multifile = 0;
	return metadata;
}
