#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "bmptools.h"

#define MIN_ARGS 2

int main(int argc, char *argv[])
{
	if(MIN_ARGS > argc)
	{
		printf("err\n");
		exit(EXIT_FAILURE);
	}
	
	int fd;
	if((fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
	{
		printf("Error creating output file\n");
		exit(EXIT_FAILURE);
	}
	
	uint8_t **pixel_data;
	int32_t width = 512;
	int32_t height = 512;
	
	pixel_data = (uint8_t**)malloc(height * sizeof(uint8_t*));
	int32_t i, j;
	for(i = 0;i < height;i++)
		pixel_data[i] = (uint8_t*)malloc(width * sizeof(uint8_t));
	
	for(i = 0;i < height;i++)
		for(j = 0;j < width;j++)
			pixel_data[i][j] = (((i * j) % 16 == 0) ? 255 : 127);
		
	writeImage(fd, pixel_data, width, height);
	
	free(pixel_data);
	close(fd);
	
	return 0;
}
