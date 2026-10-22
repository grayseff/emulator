#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ppc.h"

// static uint16_t
// be16(const unsigned char *p)
// {
// 	return
// }

int
pefload(const char *path)
{
	FILE *file;

	file = fopen(path, "rb");
	if (file == NULL) {
		perror(path);
		return -1;
	}

	fclose(file);
	return 0;
}
