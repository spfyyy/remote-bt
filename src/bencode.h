#ifndef BENCODE_H
#define BENCODE_H

/*
Utilities for parsing bencoded binary data.
*/

#include "types.h"

i32 bencode_read_string(u8_array bencoded_array, i32 offset, u8_array *out_array);
i32 bencode_read_number(u8_array bencoded_array, i32 offset, u8_array *out_array);
i32 bencode_read_list(u8_array bencoded_array, i32 offset, u8_array *out_array);
i32 bencode_read_dictionary(u8_array bencoded_array, i32 offset, u8_array *out_array);
i32 bencode_read_next(u8_array bencoded_array, i32 offset, u8_array *out_array);
char *bencode_allocate_string_value(u8_array bencoded_value);
int bencode_get_number_value(u8_array in_bencoded_value, i64 *out_value);
i32 bencode_get_value_for_key(u8_array bencoded_dictionary, char *key, size_t key_length, u8_array *out_value);

#endif

