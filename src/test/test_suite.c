#include "test_bencode.c"

int32_t main()
{
	size_t num_errors = 0;
	num_errors += test_bencode();
	return num_errors;
}
