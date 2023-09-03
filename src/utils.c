#include "stdio.h"
#include "types.h"

char *remote_bt_allocate_formatted_string(char *format, ...)
{
	va_list arglist;
	va_start(arglist, format);

	i32 byte_count = vsnprintf(NULL, 0, format, arglist);
	if (byte_count < 0)
	{
		fprintf(stderr, "error allocating string with format: %s\n", format);
		va_end(arglist);
		return NULL;
	}

	size_t buffer_size = byte_count + 1;
	char *buffer = (char *)calloc(buffer_size, sizeof(char));
	if (buffer == NULL)
	{
		fprintf(stderr, "error allocating memory while allocating string with format: %s\n", format);
		va_end(arglist);
		return NULL;
	}

	if (vsnprintf(buffer, buffer_size, format, arglist) < 0)
	{
		fprintf(stderr, "error writing string to buffer with format: %s\n", format);
		free(buffer);
		va_end(arglist);
		return NULL;
	}

	va_end(arglist);
	return buffer;
}
