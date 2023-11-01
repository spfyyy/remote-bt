#ifndef BENCODE_H
#define BENCODE_H

/*
Utilities for parsing bencoded binary data.
*/

#include "types.h"

int32_t bencode_read_string(u8_array bencoded_array, int32_t offset, u8_array *out_array);
int32_t bencode_read_number(u8_array bencoded_array, int32_t offset, u8_array *out_array);
int32_t bencode_read_list(u8_array bencoded_array, int32_t offset, u8_array *out_array);
int32_t bencode_read_dictionary(u8_array bencoded_array, int32_t offset, u8_array *out_array);
int32_t bencode_read_next(u8_array bencoded_array, int32_t offset, u8_array *out_array);
char *bencode_allocate_string_value(u8_array bencoded_value);
int bencode_get_number_value(u8_array in_bencoded_value, int64_t *out_value);
int32_t bencode_get_value_for_key(u8_array bencoded_dictionary, char *key, size_t key_length, u8_array *out_value);

#endif
