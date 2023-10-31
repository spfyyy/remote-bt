#include <stdio.h>
#include "remote_bt.h"

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "no torrent link specified\n");
		return 1;
	}

	char *link = argv[1];

	int ret_value = remote_bt_init();
	if (ret_value == 0)
	{
		ret_value = remote_bt_download(link);
	}

	remote_bt_shutdown();
	return ret_value;
}
