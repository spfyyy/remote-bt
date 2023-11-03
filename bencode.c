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

bencode_dictionary *bencode_allocate_dictionary(bencode_data in_data)
{
	size_t dict_size;
	if (bencode_size_dictionary(in_data.data, in_data.size, &dict_size) != 0)
	{
		fprintf(stderr, "could not calculate size of dictionary\n");
		return NULL;
	}

	if (dict_size != in_data.size)
	{
		fprintf(stderr, "calculated dictionary size differs from given data size\n");
		return NULL;
	}

	bencode_pair **pairs;
	size_t count = 0;
	uint8_t *data = in_data.data+1;
	size_t max_size = in_data.size-1;
	while (data[0] != 'e')
	{
		size_t key_size;
		bencode_size_string(data, max_size, &key_size);
		bencode_string *key = bencode_allocate_string((bencode_data){data,key_size});
		if (key == NULL)
		{
			fprintf(stderr, "could not allocate memory for dictionary key\n");
			goto free_existing_pairs;
		}

		data += key_size;
		max_size -= key_size;

		size_t value_size;
		bencode_size_data(data, max_size, &value_size);
		bencode_data *value = bencode_allocate_data((bencode_data){data,value_size});
		if (value == NULL)
		{
			fprintf(stderr, "could not allocate memory for dictionary value\n");
			goto free_key;
		}

		bencode_pair *new_pair = (bencode_pair *)calloc(1, sizeof(bencode_pair));
		if (new_pair == NULL)
		{
			fprintf(stderr, "could not allocate memory for dictionary pair\n");
			goto free_value;
		}

		new_pair->key = key;
		new_pair->value = value;

		bencode_pair **new_pairs = (bencode_pair **)calloc(count+1, sizeof(bencode_pair *));
		if (new_pairs == NULL)
		{
			fprintf(stderr, "could not allocate memory for dictionary pairs list\n");
			goto free_pair;
		}

		if (count == 0)
		{
			new_pairs[0] = new_pair;
		}
		else
		{
			for (int i = 0; i < count; ++i)
			{
				new_pairs[i] = pairs[i];
			}
			new_pairs[count] = new_pair;
			free(pairs);
		}

		pairs = new_pairs;
		++count;
		data += value_size;
		max_size -= value_size;
		continue;

		free_pair:
		bencode_free_pair(new_pair);
		free_value:
		bencode_free_data(value);
		free_key:
		bencode_free_string(key);
		free_existing_pairs:
		for (int i = 0; i < count; ++i)
		{
			bencode_free_pair(pairs[i]);
		}
		free(pairs);
		return NULL;
	}

	bencode_dictionary *dict = (bencode_dictionary *)calloc(1, sizeof(bencode_dictionary));
	if (dict == NULL)
	{
		fprintf(stderr, "could not allocate memory for dictionary\n");
		goto free_pairs;
	}

	bencode_data *raw = bencode_allocate_data(in_data);
	if (raw == NULL)
	{
		fprintf(stderr, "could not allocate memory for dictionary data\n");
		goto free_dict;
	}

	dict->raw = raw;
	dict->values = pairs;
	dict->count = count;
	return dict;

	free_dict:
	free(dict);
	free_pairs:
	for (int i = 0; i < count; ++i)
	{
		bencode_free_pair(pairs[i]);
	}
	free(pairs);
	return NULL;
}

bencode_string *bencode_allocate_string(bencode_data in_data)
{
	size_t string_size;
	if (bencode_size_string(in_data.data, in_data.size, &string_size) != 0)
	{
		fprintf(stderr, "could not calculate size of string\n");
		return NULL;
	}

	if (string_size != in_data.size)
	{
		fprintf(stderr, "calculated string size differs from given data size\n");
		return NULL;
	}

	size_t length = 0;
	uint8_t *data = in_data.data;
	while (data[0] != ':')
	{
		int n = data[0] - '0';
		length = length * 10 + n;
		++data;
	}

	char *value = (char *)calloc(length+1, sizeof(char));
	if (value == NULL)
	{
		fprintf(stderr, "could not allocate memory for string value\n");
		return NULL;
	}

	memcpy(value, data+1, length);

	bencode_string *string = (bencode_string *)calloc(1, sizeof(bencode_string));
	if (string == NULL)
	{
		fprintf(stderr, "could not allocate memory for string\n");
		goto free_value;
	}

	bencode_data *raw = bencode_allocate_data(in_data);
	if (raw == NULL)
	{
		fprintf(stderr, "could not allocate memory for string data\n");
		goto free_string;
	}

	string->raw = raw;
	string->value = value;
	string->length = length;
	return string;

	free_string:
	free(string);
	free_value:
	free(value);
	return NULL;
}

bencode_integer *bencode_allocate_integer(bencode_data in_data)
{
	size_t integer_size;
	if (bencode_size_integer(in_data.data, in_data.size, &integer_size) != 0)
	{
		fprintf(stderr, "could not calculate size of integer\n");
		return NULL;
	}

	if (integer_size != in_data.size)
	{
		fprintf(stderr, "calculated integer size differs from given data size\n");
		return NULL;
	}

	int64_t value = 0;
	uint8_t *data = in_data.data+1;
	while (data[0] != 'e')
	{
		int n = data[0] - '0';
		value = value * 10 + n;
		++data;
	}

	bencode_integer *integer = (bencode_integer *)calloc(1, sizeof(bencode_integer));
	if (integer == NULL)
	{
		fprintf(stderr, "could not allocate memory for integer\n");
		return NULL;
	}

	bencode_data *raw = bencode_allocate_data(in_data);
	if (raw == NULL)
	{
		fprintf(stderr, "could not allocate memory for string data\n");
		goto free_integer;
	}

	integer->raw = raw;
	integer->value = value;
	return integer;

	free_integer:
	free(integer);
	return NULL;
}

bencode_data *bencode_allocate_data(bencode_data in_data)
{
	bencode_data *data = (bencode_data *)calloc(1, sizeof(bencode_data));
	if (data == NULL)
	{
		return NULL;
	}

	data->size = in_data.size;
	data->data = (uint8_t *)calloc(data->size, sizeof(uint8_t));
	if (data->data == NULL)
	{
		free(data);
		return NULL;
	}

	memcpy(data->data, in_data.data, data->size);
	return data;
}

void bencode_free_data(bencode_data *in_data)
{
	free(in_data->data);
	free(in_data);
}

void bencode_free_string(bencode_string *in_string)
{
	bencode_free_data(in_string->raw);
	free(in_string->value);
	free(in_string);
}

void bencode_free_integer(bencode_integer *in_integer)
{
	bencode_free_data(in_integer->raw);
	free(in_integer);
}

void bencode_free_list(bencode_list *in_list)
{
	bencode_free_data(in_list->raw);
	for (int i = 0; i < in_list->count; ++i)
	{
		bencode_free_data(in_list->values[i]);
	}
	free(in_list->values);
	free(in_list);
}

void bencode_free_dictionary(bencode_dictionary *in_dictionary)
{
	bencode_free_data(in_dictionary->raw);
	for (int i = 0; i < in_dictionary->count; ++i)
	{
		bencode_free_pair(in_dictionary->values[i]);
	}
	free(in_dictionary->values);
	free(in_dictionary);
}

void bencode_free_pair(bencode_pair *in_pair)
{
	bencode_free_string(in_pair->key);
	bencode_free_data(in_pair->value);
	free(in_pair);
}

int bencode_get_pair_with_key(bencode_dictionary in_dictionary, char *in_key, size_t in_keylen, bencode_pair *out_pair)
{
	for (int i = 0; i < in_dictionary.count; ++i)
	{
		bencode_pair *pair = in_dictionary.values[i];
		if (pair->key->length != in_keylen)
		{
			continue;
		}

		char *key = pair->key->value;
		int32_t is_match = 1;
		for (int c = 0; c < in_keylen; ++c)
		{
			if (key[c] != in_key[c])
			{
				is_match = 0;
				break;
			}
		}

		if (is_match)
		{
			*out_pair = *pair;
			return 0;
		}
	}

	return 1;
}

char *bencode_allocate_string_copy(bencode_string in_string)
{
	char *copy = (char *)calloc(in_string.length+1, sizeof(char));
	if (copy == NULL)
	{
		fprintf(stderr, "could not allocate memory for string copy\n");
		return NULL;
	}

	memcpy(copy, in_string.value, in_string.length);
	return copy;
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
