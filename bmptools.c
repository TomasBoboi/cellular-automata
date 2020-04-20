#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "bmptools.h"

typedef struct __attribute__((__packed__)){
	uint16_t	fileType;
	uint32_t	fileSize;
	uint16_t	reserved1;
	uint16_t	reserved2;
	uint32_t	pixelOffset;
} bitmapFileHeader;

typedef struct __attribute__((__packed__)){
	uint32_t	infoHeaderSize;
	int32_t		bitmapWidth;
	int32_t		bitmapHeight;
	uint16_t	colorPlanes;
	uint16_t	colorDepth;
	uint32_t	compressionMethod;
	uint32_t	rawImageSize;
	int32_t		horizontalResolution;
	int32_t		verticalResolution;
	uint32_t	colorPalette;
	uint32_t	importantColors;
} bitmapInfoHeader;

uint16_t littleEndianToBigEndian_u16(uint16_t n)
{
	uint8_t hi = (n) >> 8;
	uint8_t lo = (n) & 0xFF;
	
	return (lo << 8) | hi;
}

uint32_t littleEndianToBigEndian_u32(uint32_t n)
{
	uint16_t hi = (n) >> 16;
	uint16_t lo = (n) & 0x0000FFFF;
	
	hi = littleEndianToBigEndian_u16(hi);
	lo = littleEndianToBigEndian_u16(lo);
	
	return (lo << 16) | hi;
}

void errorMessage(char *err)
{
	printf("Error: %s\n", err);
	exit(EXIT_FAILURE);
}

void writeHeader(int outputFileDescriptor, int32_t width, int32_t height)
{
	printf("Writing bitmap file header... ");

	bitmapFileHeader bfh;
	
	bfh.fileType	=	littleEndianToBigEndian_u16(BITMAP_FILE_TYPE);
	bfh.fileSize	=	width * height + BITMAP_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + (BITMAP_PALETTE_SIZE * 4);
	bfh.reserved1	=	0x0000;
	bfh.reserved2	=	0x0000;
	bfh.pixelOffset	=	BITMAP_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + BITMAP_PALETTE_SIZE;
	
	if(write(outputFileDescriptor, &bfh, sizeof(bfh)) < 0)
		errorMessage("writing the bitmap file header");
	
	printf("done\n");
}

void writeInfoHeader(int outputFileDescriptor, int32_t width, int32_t height)
{
	printf("Writing bitmap information header... ");

	bitmapInfoHeader bih;
	
	bih.infoHeaderSize			=	BITMAP_INFO_HEADER_SIZE;
	bih.bitmapWidth				=	width;
	bih.bitmapHeight			=	height;
	bih.colorPlanes				=	COLOR_PLANES;
	bih.colorDepth				=	COLOR_DEPTH;
	bih.compressionMethod		=	COMPRESSION_METHOD;
	bih.rawImageSize			=	width * height * COLOR_DEPTH / 8;
	bih.horizontalResolution	=	HORIZONTAL_RESOLUTION;
	bih.verticalResolution		=	VERTICAL_RESOLUTION;
	bih.colorPalette			=	COLOR_PALETTE;
	bih.importantColors			=	IMPORTANT_COLORS;
	
	if(write(outputFileDescriptor, &bih, sizeof(bih)) < 0)
		errorMessage("writing the bitmap information header");
	
	printf("done\n");
}

void writeColorPalette(int outputFileDescriptor)
{
	printf("Writing the color palette... ");

	uint32_t color, actualColor;
	
	for(color = 0; color < 256; color++)
	{
		actualColor = color;	actualColor <<= 8;
		actualColor |= color;	actualColor <<= 8;
		actualColor |= color;	actualColor <<= 8;
		actualColor &= 0xFFFFFF00;
		actualColor = littleEndianToBigEndian_u32(actualColor);
		
		if(write(outputFileDescriptor, &actualColor, 4) < 0)
			errorMessage("writing the color palette to the output file");
	}

	printf("done\n");
}

void writeImage(int outputFileDescriptor, uint8_t **pixelData, int32_t width, int32_t height)
{
	printf("Writing the image to the output file...\n");
	int32_t row, column;
	int32_t unpaddedWidth = width;

	while(width % 4 != 0) width++;
	for(column = unpaddedWidth; column < width; column++)
		for(row = 0; row < height; row++)
			pixelData[row][column] = COLOR_WHITE;
	
	printf("\t");
	writeHeader(outputFileDescriptor, unpaddedWidth, height);

	printf("\t");
	writeInfoHeader(outputFileDescriptor, unpaddedWidth, height);

	printf("\t");
	writeColorPalette(outputFileDescriptor);
	
	for(row = height - 1; row >= 0; row--)
	{
		if(row == height / 4)
			printf("\t25%%\n");
		else if(row == height / 2)
			printf("\t50%%\n");
		else if(row == 3 * height / 4)
			printf("\t75%%\n");
		
		for(column = 0; column < width; column++)
			if(write(outputFileDescriptor, &pixelData[row][column], sizeof(pixelData[row][column])) < 0)
				errorMessage("writing the pixel data to the output file");
	}
	printf("\t100%%\n");
}
