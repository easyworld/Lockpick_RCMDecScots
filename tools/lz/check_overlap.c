#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lz.h"

#define IPL_REGION_SIZE 0x30000

typedef struct
{
	uint8_t *data;
	uint32_t size;
} file_data_t;

static uint32_t align4(uint32_t value)
{
	return (value + 3) & ~3;
}

static file_data_t read_file(const char *path)
{
	file_data_t file = {0};
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return file;

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	if (size <= 0 || size > UINT32_MAX)
	{
		fclose(fp);
		return file;
	}

	file.data = malloc(size);
	file.size = size;
	if (!file.data || fread(file.data, 1, file.size, fp) != file.size)
	{
		free(file.data);
		file.data = NULL;
		file.size = 0;
	}
	fclose(fp);
	return file;
}

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s uncompressed part0 part1\n", argv[0]);
		return 1;
	}

	file_data_t expected = read_file(argv[1]);
	file_data_t part0 = read_file(argv[2]);
	file_data_t part1 = read_file(argv[3]);
	uint8_t *memory = calloc(1, IPL_REGION_SIZE);
	if (!expected.data || !part0.data || !part1.data || !memory)
	{
		fprintf(stderr, "Failed to read overlap-check inputs.\n");
		return 1;
	}

	uint32_t payload_size = align4(part0.size) + part1.size;
	if (align4(payload_size) > IPL_REGION_SIZE || expected.size > IPL_REGION_SIZE)
	{
		fprintf(stderr, "Payload does not fit in the loader IRAM region.\n");
		return 1;
	}

	uint32_t input = IPL_REGION_SIZE - align4(payload_size);
	memcpy(memory + input, part0.data, part0.size);
	memcpy(memory + input + align4(part0.size), part1.data, part1.size);

	uint32_t first = LZ_Uncompress(memory + input, memory, part0.size);
	uint32_t second = LZ_Uncompress(memory + input + align4(part0.size), memory + first, part1.size);
	uint32_t total = first + second;
	if (total != expected.size || memcmp(memory, expected.data, expected.size))
	{
		fprintf(stderr, "Loader overlap check failed after %u + %u bytes.\n", first, second);
		return 1;
	}

	printf("Loader overlap check: %u + %u = %u Bytes OK\n", first, second, total);
	free(memory);
	free(part1.data);
	free(part0.data);
	free(expected.data);
	return 0;
}
