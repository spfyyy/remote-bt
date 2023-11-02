#ifndef BENCODE_H
#define BENCODE_H

#include <stdint.h>

/*
Utilities for parsing bencoded binary data.
*/

typedef struct bencode_data
{
	uint8_t *data;
	size_t size;
} bencode_data;

typedef struct bencode_string
{
	bencode_data *raw;
	char *value;
	size_t length;
} bencode_string;

typedef struct bencode_integer
{
	bencode_data *raw;
	int64_t value;
} bencode_integer;

typedef struct bencode_list
{
	bencode_data *raw;
	bencode_data **values;
	size_t count;
} bencode_list;

typedef struct bencode_pair
{
	bencode_string *key;
	bencode_data *value;
} bencode_pair;

typedef struct bencode_dictionary
{
	bencode_data *raw;
	bencode_pair **values;
	size_t count;
} bencode_dictionary;

bencode_data *bencode_allocate_data(bencode_data in_data);
bencode_string *bencode_allocate_string(bencode_data in_data);
bencode_integer *bencode_allocate_integer(bencode_data in_data);
bencode_list *bencode_allocate_list(bencode_data in_data);
bencode_dictionary *bencode_allocate_dictionary(bencode_data in_data);

int bencode_get_pair_with_key(bencode_dictionary in_dictionary, char *in_key, size_t in_keylen, bencode_pair *out_pair);
char *bencode_allocate_string_copy(bencode_string in_string);

void bencode_free_data(bencode_data *in_data);
void bencode_free_string(bencode_string *in_string);
void bencode_free_integer(bencode_integer *in_integer);
void bencode_free_list(bencode_list *in_list);
void bencode_free_dictionary(bencode_dictionary *in_dictionary);
void bencode_free_pair(bencode_pair *in_pair);

#endif
