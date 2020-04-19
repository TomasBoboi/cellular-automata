#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "bmptools.h"

typedef struct __attribute__((__packed__)){
	uint16_t	file_type;
	uint32_t	file_size;
	uint16_t	reserved1;
	uint16_t	reserved2;
	uint32_t	pixel_offset;
} bitmap_file_header;

typedef struct __attribute__((__packed__)){
	uint32_t	header_size;
	int32_t		bitmap_width;
	int32_t		bitmap_height;
	uint16_t	color_planes;
	uint16_t	color_depth;
	uint32_t	compression_method;
	uint32_t	image_size;
	int32_t		horizontal_resolution;
	int32_t		vertical_resolution;
	uint32_t	color_palette;
	uint32_t	important_colors;
} BITMAPINFOHEADER;

uint16_t lite2bige_16(uint16_t n)
{
	uint8_t hi = (n) >> 8;
	uint8_t lo = (n) & 0xFF;
	
	return (lo << 8) | hi;
}

uint32_t lite2bige_32(uint32_t n)
{
	uint16_t hi = (n) >> 16;
	uint16_t lo = (n) & 0x0000FFFF;
	
	hi = lite2bige_16(hi);
	lo = lite2bige_16(lo);
	
	return (lo << 16) | hi;
}

void writeHeader(int fd, int32_t width, int32_t height)
{
	bitmap_file_header bfh;
	
	bfh.file_type		=	lite2bige_16(BITMAP_FILE_TYPE);
	bfh.file_size		=	width * height + BITMAP_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + (BITMAP_PALETTE_SIZE * 4);
	bfh.reserved1		=	0x0000;
	bfh.reserved2		=	0x0000;
	bfh.pixel_offset	=	BITMAP_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + BITMAP_PALETTE_SIZE;
	
	if(write(fd, &bfh, sizeof(bfh)) < 0)
	{
		printf("Error writing the bitmap file header\n");
		exit(EXIT_FAILURE);
	}
}

void writeInfoHeader(int fd, int32_t width, int32_t height)
{
	BITMAPINFOHEADER bih;
	
	bih.header_size				=	BITMAP_INFO_HEADER_SIZE;
	bih.bitmap_width			=	width;
	bih.bitmap_height			=	height;
	bih.color_planes			=	COLOR_PLANES;
	bih.color_depth				=	COLOR_DEPTH;
	bih.compression_method		=	COMPRESSION_METHOD;
	bih.image_size				=	width * height * COLOR_DEPTH / 8;
	bih.horizontal_resolution	=	HORIZONTAL_RESOLUTION;
	bih.vertical_resolution		=	VERTICAL_RESOLUTION;
	bih.color_palette			=	COLOR_PALETTE;
	bih.important_colors		=	IMPORTANT_COLORS;
	
	if(write(fd, &bih, sizeof(bih)) < 0)
	{
		printf("Error writing the bitmap file header\n");
		exit(EXIT_FAILURE);
	}
}

void writeColorPalette(int fd)
{
	uint16_t color;
	uint32_t actual_color;
	
	for(color = 0;color < 256;color++)
	{
		actual_color = color;
		actual_color <<= 8;
		actual_color |= color;
		actual_color <<= 8;
		actual_color |= color;
		actual_color <<= 8;
		actual_color &= 0xFFFFFF00;
		actual_color = lite2bige_32(actual_color);
		
		if(write(fd, &actual_color, 4) < 0)
		{
			printf("Error writing the color palette to the output file\n");
			exit(EXIT_FAILURE);
		}
	}
}

void writeImage(int fd, uint8_t **pixel_data, int32_t width, int32_t height)
{
	while(width % 4 != 0)
		width++;
	
	writeHeader(fd, width, height);
	writeInfoHeader(fd, width, height);
	writeColorPalette(fd);
	
	int32_t row, column;
	
	for(row = height - 1; row >= 0; row--)
		for(column = 0; column < width; column++)
			if(write(fd, &pixel_data[row][column], sizeof(pixel_data[row][column])) < 0)
			{
				printf("Error writing the pixel data to the output file\n");
				exit(EXIT_FAILURE);
			}
}
