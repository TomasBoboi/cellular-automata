#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "bmptools.h"

#define TRUE	1u
#define FALSE	0u

#define U8_ARR_LENGTH(arr) (uint8_t)((sizeof(arr))/(sizeof((arr)[0])))

#define DEFAULT_WIDTH		100
#define DEFAULT_HEIGHT		100
#define DEFAULT_RULE		30

#define FIRST_ROW		0
#define FIRST_COLUMN	0

typedef struct{
	uint8_t	force_width;
	int32_t	f_width;

	uint8_t	force_height;
	int32_t	f_height;

	uint8_t	force_output_name;
	char	*f_output_file;

	uint8_t	force_iterations;
	uint8_t f_iterations;

	uint8_t force_seed;
	char	seedPos;
	char	*f_seed;

	uint8_t	force_rule;
	uint8_t	f_rule;
} config_t;

void errorMessage(char *err);

void initialize(int argc, char *argv[]);
void parseParameters(int argc, char *argv[]);
void determineRule();
void determineImageDimensions();
void determineSeed();
void determineIterations();
void allocatePixelData();
void initializePixelData();
void determineOutputFileName();
void openOutputFile();

void	runSimulation();
uint8_t	computeCell(int32_t row, int32_t column);
uint8_t	computeRuleResult(uint8_t northWest, uint8_t north, uint8_t northEast);

void terminate();
void closeOutputFile();
void freeMemory();

config_t	configuration = {0};
int32_t		width, height;
char		*outputFileName;
int			outputFileDescriptor;
uint32_t	iterations;
uint8_t		*seed;
int32_t		seedLength;
uint8_t		**pixelData;
uint8_t		rule[8];

int main(int argc, char *argv[])
{	
	initialize(argc, argv);

	runSimulation();

	writeImage(outputFileDescriptor, pixelData, width, height);

	terminate();
	
	return 0;
}

/****** BEGIN INITIALIZE ******/
void initialize(int argc, char *argv[])
{
	printf("Initializing... \n");

	printf("\t");
	parseParameters(argc, argv);
	
	printf("\t");
	determineRule();

	printf("\t");
	determineImageDimensions();

	printf("\t");
	determineSeed();

	printf("\t");
	determineIterations();

	printf("\t");
	allocatePixelData();

	printf("\t");
	initializePixelData();

	printf("\t");
	determineOutputFileName();

	printf("\t");
	openOutputFile();
}

void parseParameters(int argc, char *argv[])
{
	printf("Parsing the parameters... ");

	uint8_t arg_index;

	for(arg_index = 1; arg_index < argc; arg_index += 2)
		switch(argv[arg_index][1])
		{
			case 'w':
				configuration.force_width = TRUE;
				configuration.f_width = atoi(argv[arg_index + 1]);
				break;
			case 'h':
				configuration.force_height = TRUE;
				configuration.f_height = atoi(argv[arg_index + 1]);
				break;
			case 'o':
				configuration.force_output_name = TRUE;
				configuration.f_output_file = argv[arg_index + 1];
				break;
			case 'i':
				configuration.force_iterations = TRUE;
				configuration.f_iterations = atoi(argv[arg_index + 1]);
				break;
			case 's':
				switch(argv[arg_index + 1][0])
				{
					case 'l': case 'L':
						configuration.seedPos = 'l';
						break;
					case 'r': case 'R':
						configuration.seedPos = 'r';
						break;
					case 'c': case 'C':
						configuration.seedPos = 'c';
						break;
					default:
						configuration.force_seed = TRUE;
						configuration.f_seed = argv[arg_index + 1];
						break;
				}
				break;
			case 'r':
				configuration.force_rule = TRUE;
				configuration.f_rule = atoi(argv[arg_index + 1]);
				break;
			default:
				errorMessage("invalid parameter");
				break;
		}
	
	printf("done\n");
}

void determineRule()
{
	printf("Determining the rule... ");

	uint8_t currentRule;
	if(TRUE == configuration.force_rule)
		currentRule = configuration.f_rule;
	else
		currentRule = DEFAULT_RULE;

	for(int32_t count = 7; count >= 0; count--)
	{
		rule[count] = (currentRule % 2 != 0) ? COLOR_BLACK : COLOR_WHITE;
		currentRule /= 2;
	}

	printf("done\n");
}

void determineImageDimensions()
{
	printf("Determining the image dimensions... ");

	if(TRUE == configuration.force_width)
		width = configuration.f_width;
	else
		width = DEFAULT_WIDTH;
	
	if(TRUE == configuration.force_height)
		height = configuration.f_height;
	else
		height = DEFAULT_HEIGHT;

	printf("done\n");
}

void determineSeed()
{
	printf("Determining the seed... ");

	int32_t index;

	if(TRUE == configuration.force_seed)
	{
		seed = (uint8_t *)malloc(strlen(configuration.f_seed) * sizeof(uint8_t));
		seedLength = strlen(configuration.f_seed);

		for(index = 0; index < strlen(configuration.f_seed); index++)
			seed[index] = (configuration.f_seed[index] == '0') ? COLOR_WHITE : COLOR_BLACK;
	}
	else
	{
		seed = (uint8_t *)malloc(width * sizeof(uint8_t));
		seedLength = width;

		for(index = 0; index < width; index++)
			if(	('l' == configuration.seedPos && FIRST_COLUMN == index) || 
				('r' == configuration.seedPos && width - 1 == index) || 
				('l' != configuration.seedPos && 'r' != configuration.seedPos && ((width + 1) / 2) == index))
				seed[index] = COLOR_BLACK;
			else
				seed[index] = COLOR_WHITE;
	}
	
	printf("done\n");
}

void determineIterations()
{
	printf("Determining iteration number... ");

	if(TRUE == configuration.force_iterations)
		iterations = configuration.f_iterations;
	else
		iterations = height;

	printf("done\n");
}

void allocatePixelData()
{
	printf("Allocating pixel data... ");

	pixelData = (uint8_t **)malloc(height * sizeof(uint8_t *));

	for(int32_t row = 0; row < height; row++)
		pixelData[row] = (uint8_t *)malloc(width * sizeof(uint8_t));
	
	printf("done\n");
}

void initializePixelData()
{
	printf("Initializing pixel data... ");

	int32_t row, column;

	for(row = 0; row < height; row++)
		for(column = 0; column < width; column++)
			pixelData[row][column] = COLOR_WHITE;
	
	for(column = 0; column < seedLength; column++)
		pixelData[FIRST_ROW][column] = seed[column];
	
	printf("done\n");
}

void determineOutputFileName()
{
	printf("Determining output file name... ");

	if(TRUE == configuration.force_output_name)
	{
		outputFileName = (char *)malloc(strlen(configuration.f_output_file) * sizeof(char));

		outputFileName = configuration.f_output_file;
	}
	else
	{
		outputFileName = (char *)malloc(18 * sizeof(char));
		strcpy(outputFileName, "output_");

		time_t currentTime = time(0);
		struct tm* currentTimeInfo = localtime(&currentTime);
		char buf[7];
		sprintf(buf, "%02d%02d%02d", currentTimeInfo -> tm_hour, currentTimeInfo -> tm_min, currentTimeInfo -> tm_sec);
		strcat(outputFileName, buf);

		strcat(outputFileName, ".bmp");
	}

	printf("done\n");
}

void openOutputFile()
{
	printf("Creating output file... ");

	if((outputFileDescriptor = open(outputFileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		errorMessage("creating output file");
	
	printf("done\n");
}
/****** END INITIALIZE ******/

/****** BEGIN SIMULATION ******/
void runSimulation()
{
	printf("Running simulation...\n");
	int32_t row, column;

	for(row = 1; row < iterations; row++)
	{
		if(row == iterations / 4)
			printf("\t25%%\n");
		else if(row == iterations / 2)
			printf("\t50%%\n");
		else if(row == 3 * iterations / 4)
			printf("\t75%%\n");
		
		for(column = 0; column < width; column++)
			pixelData[row][column] = computeCell(row, column);
	}
	printf("\t100%%\n");
}

uint8_t computeCell(int32_t row, int32_t column)
{
	if(FIRST_COLUMN == column)
		return computeRuleResult(pixelData[row - 1][width - 1], pixelData[row - 1][column], pixelData[row - 1][column + 1]);
	else if(column == width - 1)
		return computeRuleResult(pixelData[row - 1][column - 1], pixelData[row - 1][column], pixelData[row - 1][FIRST_COLUMN]);
	else
		return computeRuleResult(pixelData[row - 1][column - 1], pixelData[row - 1][column], pixelData[row - 1][column + 1]);
}

uint8_t computeRuleResult(uint8_t northWest, uint8_t north, uint8_t northEast)
{
	uint8_t index = 0;

	if(COLOR_WHITE != northEast)	index |= 1;
	if(COLOR_WHITE != north)		index |= 2;
	if(COLOR_WHITE != northWest)	index |= 4;
	
	return rule[7 - index];
}
/****** END SIMULATION ******/

/****** BEGIN TERMINATE ******/
void terminate()
{
	printf("Terminating program... \n");

	printf("\t");
	closeOutputFile();

	printf("\t");
	freeMemory();
}

void closeOutputFile()
{
	printf("Closing output file... ");

	if(close(outputFileDescriptor) < 0)
		errorMessage("closing output file");
	
	printf("done\n");
}

void freeMemory()
{
	printf("Freeing allocated memory... ");

	//free(outputFileName);
	free(seed);
	free(pixelData);

	printf("done\n");
}
/****** END TERMINATE ******/