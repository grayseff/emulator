#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppc.h"

struct pefsection {
	uint32_t name;
	uint32_t address;
	uint32_t totalsize;
	uint32_t unpackedsize;
	uint32_t packedsize;
	uint32_t offset;
	uint8_t kind;
	uint8_t share;
	uint8_t alignment;
};

static uint16_t bigend16(const unsigned char *);
static uint16_t bigend32(const unsigned char *);

int
pefload(const char *path, Memory *mem)
{
	FILE *file = NULL;
	struct pefsection *table = NULL;
	unsigned char header[40];
	unsigned char section[28];

	uint16_t sections;
	uint16_t instantiated;
	int status = -1;

	file = fopen(path, "rb");
	if (file == NULL) {
		perror(path);
		goto cleanup;
	}

	/* read in the file header */

	if (fread(header,1,sizeof header, file) != sizeof header) {
		fprintf(stderr, "PEF: failed to read header\n");
		goto cleanup;
	}
	if (memcmp(header, "Joy!peffpwpc",12) != 0) {
		fprintf(stderr, "PEF: invalid PowerPC executable\n");
		goto cleanup;
	}
	if (bigend32(header + 12) != 1) {
		fprintf(stderr, "PEF: unsupported version\n");
		goto cleanup;
	}

	sections = bigend16(header + 32);
	instantiated = bigend16(header + 34);
	
	if (!sections || instantiated > sections) {
		fprintf(stderr, "PEF: invalid section count\n");
		goto cleanup;
	}

	table = calloc(sections, sizeof *table);
	
	if (!table) {
		fprintf(stderr, "PEF: failed to allocate section table\n");
		goto cleanup;
	}

	/* read in the section headers */

	for (uint16_t i = 0; i < sections; i++) {

		if (fread(section, 1, sizeof section, file) != sizeof section) {
			fprintf(stderr, "PEF: failed to read section header\n");
			goto cleanup;
		}

		table[i].name = bigend32(section);
		table[i].sectionaddress = bigend32(section + 4 );
		table[i].totalsize      = bigend32(section + 8 );
		table[i].unpackedsize   = bigend32(section + 12);
		table[i].packedsize     = bigend32(section + 16);
		table[i].sectionoffset  = bigend32(section + 20);
		table[i].sectionkind    = section[24];
		table[i].sharekind      = section[25];
		table[i].alignment      = section[26];
		
	}

	for (uint16_t i = 0; i < instantiated; i++) {
		struct pefsection *s = &table[i];

		/* check memory is large enough */
		if (s->sectionaddress > mem->size ||
		    s->totalsize > mem->size - s->sectionaddress) {
			fprintf(stderr, "PEF: section exceeds memory\n");
			goto cleanup;
		}
		if (s->packedsize != s->unpackedsize){
			fprintf(stderr, "PEF: invalid section size\n");
			goto cleanup;
		}
		if (fseek(file, s->sectionoffset, SEEK_SET)) {
			fprintf(stderr, "PEF: failed to seek to section\n");
			goto cleanup;
		}
		/* read instructions into memory */
		switch (s->sectionkind) {
		case 0: /* code */
		case 1: /* unpacked data */
			if (s->packedsize != s->unpackedsize){
				fprintf(stderr,"PEF: invalid section size\n");
				goto cleanup;
			}
			if (fread(mem->data + s->sectionaddress,
				1, s->packedsize, file) != s->packedsize) {
				fprintf(stderr,"PEF: failed to read code section\n");
				goto cleanup;
			}
			break;
		case 2: /* pattern initialised data */
			
			break;

		default:
			fprintf(stderr,"PEF: unsupported section kind\n");
			goto cleanup;
		}
	}



	status = 0;
cleanup:
	free(table);
	if (file)
		fclose(file);
	return status;
}

static uint16_t
bigend16(const unsigned char *p)
{
	return ((uint16_t)p[0] << 8) | p[1];
}

static uint32_t
bigend32(const unsigned char *p)
{
	return ((uint32_t)p[0] << 24)
		| ((uint_32)p[1] << 16)
		| ((uint32)p[2] << 8)
		| (uint32)p[3];
}
