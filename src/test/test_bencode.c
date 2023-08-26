#include <stdio.h>

#include "../bencode.c"

u8_array bencoded_dictionary = {
	(u8 *)"d4:testli1ei23ei456ee5:test23:lole",
	34
};

void fprint_u8_array(FILE *stream, u8_array array)
{
	fprintf(stream, "{\n\tdata: %p,\n\tsize: %ld\n}\n", (void *)array.data, array.size);
}

i32 verify_u8_arrays_are_equal(u8_array actual, u8_array expected)
{
	if (actual.size == expected.size && actual.data == expected.data)
	{
		return 0;
	}
	fprintf(stderr, "expected: ");
	fprint_u8_array(stderr, expected);
	fprintf(stderr, "actual: ");
	fprint_u8_array(stderr, actual);
	return 1;
}

i32 test_extract_bencode_string()
{
	u8_array expected = {
		bencoded_dictionary.data+1,
		6
	};
	u8_array string;
	i32 read_result = bencode_read_string(bencoded_dictionary, 1, &string);
	i32 verify_result = verify_u8_arrays_are_equal(string, expected);
	if (read_result != 0 || verify_result != 0)
	{
		fprintf(stderr, "test_extract_bencode_string failed\n\n");
		return 1;
	}
	return 0;
}

i32 test_extract_bencode_number()
{
	u8_array expected = {
		bencoded_dictionary.data+15,
		5
	};
	u8_array number;
	i32 read_result = bencode_read_number(bencoded_dictionary, 15, &number);
	i32 verify_result = verify_u8_arrays_are_equal(number, expected);
	if (read_result != 0 || verify_result != 0)
	{
		fprintf(stderr, "test_extract_bencode_number failed\n\n");
		return 1;
	}
	return 0;
}

i32 test_extract_bencode_list()
{
	u8_array expected = {
		bencoded_dictionary.data+7,
		14
	};
	u8_array list;
	i32 read_result = bencode_read_list(bencoded_dictionary, 7, &list);
	i32 verify_result = verify_u8_arrays_are_equal(list, expected);
	if (read_result != 0 || verify_result != 0)
	{
		fprintf(stderr, "test_extract_bencode_list failed\n\n");
		return 1;
	}
	return 0;
}

i32 test_extract_bencode_dictionary()
{
	u8_array expected = {
		bencoded_dictionary.data,
		34
	};
	u8_array dictionary;
	i32 read_result = bencode_read_dictionary(bencoded_dictionary, 0, &dictionary);
	i32 verify_result = verify_u8_arrays_are_equal(dictionary, expected);
	if (read_result != 0 || verify_result != 0)
	{
		fprintf(stderr, "test_extract_bencode_dictionary failed\n\n");
		return 1;
	}
	return 0;
}

i32 test_allocate_string_value()
{
	u8_array bencoded_value;
	bencode_read_string(bencoded_dictionary, 1, &bencoded_value);
	char *value = bencode_allocate_string_value(bencoded_value);
	char *expected = "test";
	for (i32 i = 0; i < 5; ++i)
	{
		if (value == NULL || (value[i] != expected[i]))
		{
			if (value != NULL)
			{
				free(value);
			}
			fprintf(stderr, "test_allocate_string_value failed\n");
			return 1;
		}
	}
	free(value);
	return 0;
}

i32 test_get_value_for_key()
{
	u8_array value;
	i32 result = bencode_get_value_for_key(bencoded_dictionary, "test2", 5, &value);

	u8_array expected = {
		bencoded_dictionary.data + 28,
		5
	};

	i32 verify_result = verify_u8_arrays_are_equal(value, expected);
	if (result != 0 || verify_result != 0)
	{
		fprintf(stderr, "test_get_value_for_key failed\n\n");
		return 1;
	}
	return 0;
	
}

i32 test_bencode()
{
	i32 errors = 0;
	errors += test_extract_bencode_string();
	errors += test_extract_bencode_number();
	errors += test_extract_bencode_list();
	errors += test_extract_bencode_dictionary();
	errors += test_allocate_string_value();
	errors += test_get_value_for_key();
	if (errors == 0)
	{
		fprintf(stdout, "all bencode tests passed\n");
	}
	return errors;
}

