#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "torrent.h"

torrent_metadata *torrent_allocate_metadata_from_dictionary(bencode_dictionary in_dictionary)
{
	bencode_pair announce_pair;
	if (bencode_get_pair_with_key(in_dictionary, "announce", 8, &announce_pair) != 0)
	{
		fprintf(stderr, "could not find announce key in dictionary\n");
		return NULL;
	}

	bencode_pair info_pair;
	if (bencode_get_pair_with_key(in_dictionary, "info", 4, &info_pair) != 0)
	{
		fprintf(stderr, "could not find info key in dictionary\n");
		return NULL;
	}

	bencode_dictionary *info = bencode_allocate_dictionary(*info_pair.value);
	if (info == NULL)
	{
		fprintf(stderr, "could not allocate info dictionary\n");
		return NULL;
	}

	bencode_pair name_pair;
	if (bencode_get_pair_with_key(*info, "name", 4, &name_pair) != 0)
	{
		fprintf(stderr, "could not find name key in dictionary\n");
		goto free_info;
	}

	bencode_pair piece_length_pair;
	if (bencode_get_pair_with_key(*info, "piece length", 12, &piece_length_pair) != 0)
	{
		fprintf(stderr, "could not find piece length key in dictionary\n");
		goto free_info;
	}

	bencode_pair pieces_pair;
	if (bencode_get_pair_with_key(*info, "pieces", 6, &pieces_pair) != 0)
	{
		fprintf(stderr, "could not find pieces key in dictionary\n");
		goto free_info;
	}

	bencode_pair length_pair;
	if (bencode_get_pair_with_key(*info, "length", 6, &length_pair) != 0)
	{
		// TODO: length is optional in multifile
		fprintf(stderr, "could not find length key in dictionary\n");
		goto free_info;
	}

	bencode_string *bencode_announce = bencode_allocate_string(*announce_pair.value);
	if (bencode_announce == NULL)
	{
		fprintf(stderr, "could not allocate announce string\n");
		goto free_info;
	}

	char *announce = bencode_allocate_string_copy(*bencode_announce);
	if (announce == NULL)
	{
		fprintf(stderr, "could not copy announce\n");
		goto free_bencode_announce;
	}

	bencode_string *bencode_name = bencode_allocate_string(*name_pair.value);
	if (bencode_name == NULL)
	{
		fprintf(stderr, "could not allocate name string\n");
		goto free_announce;
	}

	char *name = bencode_allocate_string_copy(*bencode_name);
	if (name == NULL)
	{
		fprintf(stderr, "could not copy name\n");
		goto free_bencode_name;
	}

	bencode_integer *bencode_piece_length = bencode_allocate_integer(*piece_length_pair.value);
	if (bencode_piece_length == NULL)
	{
		fprintf(stderr, "could not allocate piece length integer\n");
		goto free_name;
	}

	int64_t piece_length = bencode_piece_length->value;

	bencode_string *bencode_pieces = bencode_allocate_string(*pieces_pair.value);
	if (bencode_pieces == NULL)
	{
		fprintf(stderr, "could not allocate pieces string\n");
		goto free_bencode_piece_length;
	}

	char *pieces = bencode_allocate_string_copy(*bencode_pieces);
	if (pieces == NULL)
	{
		fprintf(stderr, "could not copy pieces\n");
		goto free_bencode_pieces;
	}

	bencode_integer *bencode_length = bencode_allocate_integer(*length_pair.value);
	if (bencode_length == NULL)
	{
		fprintf(stderr, "could not allocate length integer\n");
		goto free_pieces;
	}

	int64_t length = bencode_length->value;

	torrent_metadata *result = (torrent_metadata *)calloc(1, sizeof(torrent_metadata));
	if (result == NULL)
	{
		fprintf(stderr, "could not allocate memory for torrent\n");
		goto free_bencode_length;
	}

	result->announce = announce;
	result->name = name;
	result->piece_length = piece_length;
	result->pieces = pieces;
	result->length = length;
	result->is_multifile = 0;

	bencode_free_integer(bencode_length);
	bencode_free_string(bencode_pieces);
	bencode_free_integer(bencode_piece_length);
	bencode_free_string(bencode_name);
	bencode_free_string(bencode_announce);
	bencode_free_dictionary(info);

	return result;

	free_bencode_length:
	bencode_free_integer(bencode_length);
	free_pieces:
	free(pieces);
	free_bencode_pieces:
	bencode_free_string(bencode_pieces);
	free_bencode_piece_length:
	bencode_free_integer(bencode_piece_length);
	free_name:
	free(name);
	free_bencode_name:
	bencode_free_string(bencode_name);
	free_announce:
	free(announce);
	free_bencode_announce:
	bencode_free_string(bencode_announce);
	free_info:
	bencode_free_dictionary(info);
	return NULL;
}

void torrent_free_metadata(torrent_metadata *metadata)
{
	free(metadata->announce);
	free(metadata->name);
	free(metadata->pieces);
	free(metadata);
}
