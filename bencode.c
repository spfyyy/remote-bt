#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bencode.h"

// internal utilities
static int bencode_size_data(uint8_t *in_data, size_t in_max_size, size_t *out_size);
static int bencode_size_string(uint8_t *in_data, size_t in_max_size, size_t *out_size);
static int bencode_size_integer(uint8_t *in_data, size_t in_max_size, size_t *out_size);
static int bencode_size_list(uint8_t *in_data, size_t in_max_size, size_t *out_size);
static int bencode_size_dictionary(uint8_t *in_data, size_t in_max_size, size_t *out_size);

int bencode_value_for_key(uint8_t *dictionary, size_t dictionary_size, char *key, size_t key_length, uint8_t **out_bencoded_value, size_t *out_bencoded_value_size)
{
	size_t position = 1;
	while (position < dictionary_size)
	{
		size_t key_size;
		if (bencode_size_string(dictionary+position, dictionary_size-position, &key_size) != 0)
		{
			return 1;
		}

		uint8_t *key_string;
		size_t key_string_length;
		if (bencode_parse_string(dictionary+position, key_size, &key_string, &key_string_length) != 0)
		{
			return 1;
		}

		position += key_size;
		size_t value_size;
		if (bencode_size_data(dictionary+position, dictionary_size - position, &value_size) != 0)
		{
			return 1;
		}

		int32_t keys_match = 0;
		if (key_string_length == key_length)
		{
			keys_match = 1;
			for (int i = 0; i < key_length; ++i)
			{
				if (key_string[i] != key[i])
				{
					keys_match = 0;
					break;
				}
			}
		}

		if (keys_match)
		{
			*out_bencoded_value = dictionary+position;
			*out_bencoded_value_size = value_size;
			return 0;
		}

		position += value_size;
	}

	return 1;
}

int bencode_parse_string(uint8_t *bencoded_string, size_t bencoded_string_size, uint8_t **out_string, size_t *out_string_length)
{
	if (bencoded_string_size < 2 || bencoded_string[0] < '0' || bencoded_string[0] > '9')
	{
		return 1;
	}

	size_t string_length = 0;
	size_t position = 0;
	while (position < bencoded_string_size)
	{
		char c = bencoded_string[position];
		if (c < '0' || c > '9')
		{
			if (c != ':')
			{
				return 1;
			}

			++position;
			if (position + string_length != bencoded_string_size)
			{
				return 1;
			}

			*out_string = bencoded_string + position;
			*out_string_length = string_length;
			return 0;
		}

		int n = c - '0';
		string_length = string_length * 10 + n;
		++position;
	}

	return 1;
}

int bencode_parse_integer(uint8_t *bencoded_integer, size_t bencoded_integer_size, int64_t *out_integer)
{
	if (bencoded_integer_size < 3 || bencoded_integer[0] != 'i' || bencoded_integer[bencoded_integer_size-1] != 'e')
	{
		return 1;
	}

	int64_t value = 0;
	size_t position = 1;
	while (position < bencoded_integer_size-1)
	{
		char c = bencoded_integer[position];
		if (c < '0' || c > '9')
		{
			return 1;
		}

		int n = c - '0';
		value = value * 10 + n;
		++position;
	}

	*out_integer = value;
	return 0;
}

static int bencode_size_data(uint8_t *in_data, size_t in_max_size, size_t *out_size)
{
	if (in_max_size < 1)
	{
		return 1;
	}

	char c = in_data[0];
	if (c >= '0' && c <= '9')
	{
		return bencode_size_string(in_data, in_max_size, out_size);
	}
	else if (c == 'i')
	{
		return bencode_size_integer(in_data, in_max_size, out_size);
	}
	else if (c == 'l')
	{
		return bencode_size_list(in_data, in_max_size, out_size);
	}
	else if (c == 'd')
	{
		return bencode_size_dictionary(in_data, in_max_size, out_size);
	}
	else
	{
		return 1;
	}
}

static int bencode_size_string(uint8_t *in_data, size_t in_max_size, size_t *out_size)
{
	if (in_max_size < 2 || in_data[0] < '0' || in_data[0] > '9')
	{
		return 1;
	}

	size_t num = 0;
	for (int i = 0; i < in_max_size; ++i)
	{
		char c = in_data[i];
		if (c >= '0' && c <= '9')
		{
			int n = c - '0';
			num = num * 10 + n;
			continue;
		}
		else if (c == ':')
		{
			int calc_size = i + num + 1;
			if (calc_size > in_max_size)
			{
				return 1;
			}
			*out_size = calc_size;
			return 0;
		}
		else
		{
			break;
		}
	}
	return 1;
}

static int bencode_size_integer(uint8_t *in_data, size_t in_max_size, size_t *out_size)
{
	if (in_max_size < 3 || in_data[0] != 'i')
	{
		return 1;
	}

	for (int i = 1; i < in_max_size; ++i)
	{
		char c = in_data[i];
		if (c >= '0' && c <= '9')
		{
			continue;
		}
		else if (c == 'e')
		{
			int calc_size = i + 1;
			if (calc_size > in_max_size)
			{
				return 1;
			}
			*out_size = calc_size;
			return 0;
		}
		else
		{
			break;
		}
	}
	return 1;
}

static int bencode_size_list(uint8_t *in_data, size_t in_max_size, size_t *out_size)
{
	if (in_max_size < 2 || in_data[0] != 'l')
	{
		return 1;
	}

	for (int i = 1; i < in_max_size;)
	{
		if (in_data[i] == 'e')
		{
			int calc_size = i + 1;
			if (calc_size > in_max_size)
			{
				return 1;
			}
			*out_size = calc_size;
			return 0;
		}

		size_t elem_size;
		if (bencode_size_data(in_data+i, in_max_size-i, &elem_size) != 0)
		{
			return 1;
		}

		i += elem_size;
	}

	return 1;
}

static int bencode_size_dictionary(uint8_t *in_data, size_t in_max_size, size_t *out_size)
{
	if (in_max_size < 2 || in_data[0] != 'd')
	{
		return 1;
	}

	for (int i = 1; i < in_max_size;)
	{
		if (in_data[i] == 'e')
		{
			int calc_size = i + 1;
			if (calc_size > in_max_size)
			{
				return 1;
			}
			*out_size = calc_size;
			return 0;
		}

		size_t elem_size;
		if (bencode_size_data(in_data+i, in_max_size-i, &elem_size) != 0)
		{
			return 1;
		}

		i += elem_size;

		if (bencode_size_data(in_data+i, in_max_size-i, &elem_size) != 0)
		{
			return 1;
		}

		i += elem_size;
	}

	return 1;
}
