#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bencode.h"

i32 bencode_read_string(u8_array bencoded_array, i32 offset, u8_array *out_array)
{
	if (offset >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading string\n");
		return 1;
	}

	char c = bencoded_array.data[offset];

	if (c < '0' || c > '9')
	{
		fprintf(stderr, "Expected digit while reading string but got %c\n", c);
		return 1;
	}

	size_t size = 0;
	i32 i;
	for (i = offset; i < bencoded_array.size; ++i)
	{
		c = bencoded_array.data[i];

		if (c == ':')
		{
			break;
		} else if (c < '0' || c > '9')
		{
			fprintf(stderr, "Expected digit while reading string but got %c\n", c);
			return 1;
		}

		size = (10 * size) + (c - '0');
	}

	if (c != ':')
	{
		fprintf(stderr, "Expected ':' while reading string but got %c\n", c);
		return 1;
	}

	i += size;

	if (i >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading string\n");
		return 1;
	}

	out_array->data = bencoded_array.data + offset;
	out_array->size = i - offset + 1;
	return 0;
}

i32 bencode_read_number(u8_array bencoded_array, i32 offset, u8_array *out_array)
{
	if (offset >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading number\n");
		return 1;
	}

	char c = bencoded_array.data[offset];
	if (c != 'i')
	{
		fprintf(stderr, "Expected character 'i' while reading number but got %c\n", c);
		return 1;
	}

	i32 i;
	for (i = offset + 1; i < bencoded_array.size; ++i) {
		c = bencoded_array.data[i];
		if (c == 'e')
		{
			break;
		}
		if (c < '0' || c > '9')
		{
			fprintf(stderr, "Expected digits while reading number but got %c\n", c);
			return 1;
		}
	}

	if (c != 'e')
	{
		fprintf(stderr, "Exceeded bounds while reading number\n");
		return 1;
	}

	out_array->data = bencoded_array.data + offset;
	out_array->size = i - offset + 1;
	return 0;
}

i32 bencode_read_list(u8_array bencoded_array, i32 offset, u8_array *out_array)
{
	if (offset >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading list\n");
		return 1;
	}

	char c = bencoded_array.data[offset];
	if (c != 'l')
	{
		fprintf(stderr, "Expected character 'l' while reading list but got %c\n", c);
		return 1;
	}

	i32 i;
	for (i = offset + 1; i < bencoded_array.size;) {
		c = bencoded_array.data[i];
		if (c == 'e')
		{
			break;
		}

		u8_array next;
		i32 next_result = bencode_read_next(bencoded_array, i, &next);

		if (next_result != 0)
		{
			fprintf(stderr, "Unable to parse while reading list\n");
			return 1;
		}

		i += next.size;

		if (i >= bencoded_array.size)
		{
			fprintf(stderr, "Exceeded bounds while reading list\n");
			return 1;
		}
	}

	if (c != 'e')
	{
		fprintf(stderr, "Exceeded bounds while reading list\n");
		return 1;
	}

	out_array->data = bencoded_array.data + offset;
	out_array->size = i - offset + 1;
	return 0;
}

i32 bencode_read_dictionary(u8_array bencoded_array, i32 offset, u8_array *out_array)
{
	if (offset >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading dictionary\n");
		return 1;
	}

	char c = bencoded_array.data[offset];
	if (c != 'd')
	{
		fprintf(stderr, "Expected character 'd' while reading dictionary but got %c\n", c);
		return 1;
	}

	i32 i;
	for (i = offset + 1; i < bencoded_array.size;) {
		c = bencoded_array.data[i];
		if (c == 'e')
		{
			break;
		}

		u8_array next;
		i32 next_result = bencode_read_string(bencoded_array, i, &next);

		if (next_result != 0)
		{
			fprintf(stderr, "Unable to parse while reading dictionary\n");
			return 1;
		}

		i += next.size;
		next_result = bencode_read_next(bencoded_array, i, &next);

		if (next_result != 0)
		{
			fprintf(stderr, "Unable to parse while reading dictionary\n");
			return 1;
		}

		i += next.size;

		if (i >= bencoded_array.size)
		{
			fprintf(stderr, "Exceeded bounds while reading dictionary\n");
			return 1;
		}
	}

	if (c != 'e')
	{
		fprintf(stderr, "Exceeded bounds while reading dictionary\n");
		return 1;
	}

	out_array->data = bencoded_array.data + offset;
	out_array->size = i - offset + 1;
	return 0;
}

i32 bencode_read_next(u8_array bencoded_array, i32 offset, u8_array *out_array)
{
	if (offset >= bencoded_array.size)
	{
		fprintf(stderr, "Exceeded bounds while reading next token\n");
		return 1;
	}

	char c = bencoded_array.data[offset];
	i32 next_result;
	if (c >= '0' && c <= '9')
	{
		next_result = bencode_read_string(bencoded_array, offset, out_array);
	} else if (c == 'i')
	{
		next_result = bencode_read_number(bencoded_array, offset, out_array);
	} else if (c == 'l')
	{
		next_result = bencode_read_list(bencoded_array, offset, out_array);
	} else if (c == 'd')
	{
		next_result = bencode_read_dictionary(bencoded_array, offset, out_array);
	} else
	{
		fprintf(stderr, "Expected character to be a digit, 'i', 'l', or 'd' while reading next token but got %c\n", c);
		next_result = 1;
	}

	return next_result;
}

char *bencode_allocate_string_value(u8_array bencoded_value)
{
	i32 start_index;
	for (i32 i = 0; i < bencoded_value.size; ++i)
	{
		if (bencoded_value.data[i] == ':')
		{
			start_index = i + 1;
			break;
		}

		if (i == bencoded_value.size - 1)
		{
			fprintf(stderr, "Failed to parse bencoded value as string\n");
			return NULL;
		}
	}

	if (start_index >= bencoded_value.size)
	{
		fprintf(stderr, "Failed to parse bencoded value as string\n");
		return NULL;
	}

	size_t str_len = bencoded_value.size - start_index;
	char *result = (char *)(calloc(str_len + 1, sizeof(char)));
	memcpy(result, bencoded_value.data + start_index, str_len);
	return result;
}

int bencode_get_number_value(u8_array in_bencoded_value, i64 *out_value)
{
	if (in_bencoded_value.size < 3)
	{
		fprintf(stderr, "reached end of data while parsing number\n");
		return 1;
	}

	char c = in_bencoded_value.data[0];
	if (c != 'i')
	{
		fprintf(stderr, "expected to read 'i' while parsing number but got %c\n", c);
		return 1;
	}

	i64 size = 0;
	i32 i;
	for (i = 1; i < in_bencoded_value.size-1; ++i)
	{
		c = in_bencoded_value.data[i];
		if (c < '0' || c > '9')
		{
			fprintf(stderr, "Expected digit while parsing number but got %c\n", c);
			return 1;
		}

		size = (10 * size) + (c - '0');
	}

	c = in_bencoded_value.data[i];
	if (c != 'e')
	{
		fprintf(stderr, "Expected 'e' while parsing number but got %c\n", c);
		return 1;
	}

	*out_value = size;
	return 0;
}

i32 bencode_get_value_for_key(u8_array bencoded_dictionary, char *key, size_t key_length, u8_array *out_value)
{
	if (bencoded_dictionary.size < 2 || bencoded_dictionary.data[0] != 'd')
	{
		fprintf(stderr, "Unable to process data as dictionary\n");
		return 1;
	}

	for (i32 i = 1; i < bencoded_dictionary.size && bencoded_dictionary.data[i] != 'e';)
	{
		u8_array key_array;
		if (bencode_read_string(bencoded_dictionary, i, &key_array) != 0)
		{
			fprintf(stderr, "Failed to get key from dictionary\n");
			return 1;
		}

		i += key_array.size;

		u8_array value_array;
		if (bencode_read_next(bencoded_dictionary, i, &value_array) != 0)
		{
			fprintf(stderr, "Failed to get value from dictionary\n");
			return 1;
		}

		i += value_array.size;

		i32 key_start_index;
		for (i32 i = 0; i < key_array.size; ++i)
		{
			if (key_array.data[i] == ':')
			{
				key_start_index = i + 1;
				break;
			}
		}

		if ((key_array.size - key_start_index) != key_length)
		{
			continue;
		}

		b32 match = 1;
		for (i32 j = 0; j < key_length; ++j)
		{
			if (key_array.data[j + key_start_index] != key[j])
			{
				match = 0;
				break;
			}
		}

		if (!match)
		{
			continue;
		}

		*out_value = value_array;
		return 0;
	}

	return 1;
}

