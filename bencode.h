#ifndef BENCODE_H
#define BENCODE_H

#include <stdint.h>

int bencode_value_for_key(uint8_t *dictionary, size_t dictionary_size, char *key, size_t key_length, uint8_t **out_bencoded_value, size_t *out_bencoded_value_size);
int bencode_parse_string(uint8_t *bencoded_string, size_t bencoded_string_size, uint8_t **out_string, size_t *out_string_length);
int bencode_parse_integer(uint8_t *bencoded_integer, size_t bencoded_integer_size, int64_t *out_integer);

#endif
