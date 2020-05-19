#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cella.h"
#include "bmptools.h"
#include "utils.h"

struct cella_configuration_st {
	uint8_t	forceWidth_u8;
	int32_t	forcedWidth_s32;

	uint8_t	forceHeight_u8;
	int32_t	forcedHeight_s32;

	uint8_t	forceOutputName_u8;
	char	*forcedOutputName_pc;

	uint8_t	forceIterations_u8;
	uint32_t forcedIterations_u32;

	uint8_t forceSeed_u8;
	char	seedPosition_c;
	char	*forcedSeed_pc;

	uint8_t	forceRule_u8;
	uint8_t	forcedRule_u8;
};

static void cella_ParseParameters(int argc, char *argv[]);
static void cella_DetermineRule(void);
static void cella_DetermineImageDimensions(void);
static void cella_DetermineSeed(void);
static void cella_DetermineIterations(void);
static void cella_AllocatePixelData(void);
static void cella_InitializePixelData(void);
static void cella_DetermineOutputFileName(void);
static void cella_OpenOutputFile(void);

static uint8_t cella_ComputeCell(int32_t row_s32, int32_t column_s32);
static uint8_t cella_ComputeRuleResult(uint8_t northWest_u8, uint8_t north_u8, uint8_t northEast_u8);

static void cella_CloseOutputFile(void);
static void cella_FreeMemory(void);

cella_configuration_t	cella_Configuration_t = {0};

int32_t		cella_ImageWidth_s32, cella_ImageHeight_s32;
char		*cella_OutputFileName_pc;
int			cella_OutputFileDescriptor_fd;
uint32_t	cella_NumberOfIterations_u32;
uint8_t		*cella_Seed_pu8;
int32_t		cella_SeedLength_s32;
uint8_t		**cella_PixelData_ppu8;
uint8_t		cella_Rule_au8[8];

int main(int argc, char *argv[])
{
	cella_Initialize(argc, argv);

	cella_RunSimulation();

	bmp_WriteImage(cella_OutputFileDescriptor_fd, cella_PixelData_ppu8, cella_ImageWidth_s32, cella_ImageHeight_s32);

	cella_Terminate();
	
	return 0;
}

/**************** BEGIN INITIALIZE ****************/
void cella_Initialize(int argc, char *argv[])
{
	printf("Initializing...\n");

	printf("\t");
	cella_ParseParameters(argc, argv);
	
	printf("\t");
	cella_DetermineRule();

	printf("\t");
	cella_DetermineImageDimensions();

	printf("\t");
	cella_DetermineSeed();

	printf("\t");
	cella_DetermineIterations();

	printf("\t");
	cella_AllocatePixelData();

	printf("\t");
	cella_InitializePixelData();

	printf("\t");
	cella_DetermineOutputFileName();

	printf("\t");
	cella_OpenOutputFile();
}

static void cella_ParseParameters(int argc, char *argv[])
{
	printf("Parsing the parameters... ");

	uint8_t argIndex_u8;

	for(argIndex_u8 = 1; argIndex_u8 < argc; argIndex_u8 += 2)
		switch(argv[argIndex_u8][1])
		{
			case 'w':
				cella_Configuration_t.forceWidth_u8 = TRUE;
				cella_Configuration_t.forcedWidth_s32 = atoi(argv[argIndex_u8 + 1]);
				break;
			case 'h':
				cella_Configuration_t.forceHeight_u8 = TRUE;
				cella_Configuration_t.forcedHeight_s32 = atoi(argv[argIndex_u8 + 1]);
				break;
			case 'o':
				cella_Configuration_t.forceOutputName_u8 = TRUE;
				cella_Configuration_t.forcedOutputName_pc = argv[argIndex_u8 + 1];
				break;
			case 'i':
				cella_Configuration_t.forceIterations_u8 = TRUE;
				cella_Configuration_t.forcedIterations_u32 = atoi(argv[argIndex_u8 + 1]);
				break;
			case 's':
				switch(argv[argIndex_u8 + 1][0])
				{
					case 'l': case 'L':
						cella_Configuration_t.seedPosition_c = 'l';
						break;
					case 'r': case 'R':
						cella_Configuration_t.seedPosition_c = 'r';
						break;
					case 'c': case 'C':
						cella_Configuration_t.seedPosition_c = 'c';
						break;
					default:
						cella_Configuration_t.forceSeed_u8 = TRUE;
						cella_Configuration_t.forcedSeed_pc = argv[argIndex_u8 + 1];
						break;
				}
				break;
			case 'r':
				cella_Configuration_t.forceRule_u8 = TRUE;
				cella_Configuration_t.forcedRule_u8 = atoi(argv[argIndex_u8 + 1]);
				break;
			default:
				utils_ErrorMessage("invalid parameter");
				break;
		}
	
	printf("done\n");
}

static void cella_DetermineRule(void)
{
	printf("Determining the rule... ");

	uint8_t currentRule_u8;

	if(TRUE == cella_Configuration_t.forceRule_u8)
		currentRule_u8 = cella_Configuration_t.forcedRule_u8;
	else
		currentRule_u8 = CELLA_DEFAULT_RULE;

	for(int8_t count_s8 = 7; count_s8 >= 0; count_s8--)
	{
		cella_Rule_au8[count_s8] = (currentRule_u8 % 2 != 0) ? BMP_COLOR_BLACK : BMP_COLOR_WHITE;
		currentRule_u8 /= 2;
	}

	printf("done\n");
}

static void cella_DetermineImageDimensions(void)
{
	printf("Determining the image dimensions... ");

	if(TRUE == cella_Configuration_t.forceWidth_u8)
		cella_ImageWidth_s32 = cella_Configuration_t.forcedWidth_s32;
	else
		cella_ImageWidth_s32 = CELLA_DEFAULT_WIDTH;
	
	if(TRUE == cella_Configuration_t.forceHeight_u8)
		cella_ImageHeight_s32 = cella_Configuration_t.forcedHeight_s32;
	else
		cella_ImageHeight_s32 = CELLA_DEFAULT_HEIGHT;

	printf("done\n");
}

static void cella_DetermineSeed(void)
{
	printf("Determining the seed... ");

	int32_t index_s32;

	if(TRUE == cella_Configuration_t.forceSeed_u8)
	{
		cella_SeedLength_s32 = strlen(cella_Configuration_t.forcedSeed_pc);
		cella_Seed_pu8 = (uint8_t *)malloc(cella_SeedLength_s32 * sizeof(uint8_t));

		for(index_s32 = 0; index_s32 < cella_SeedLength_s32; index_s32++)
			cella_Seed_pu8[index_s32] = (cella_Configuration_t.forcedSeed_pc[index_s32] == '0') ? BMP_COLOR_WHITE : BMP_COLOR_BLACK;
	}
	else
	{
		cella_SeedLength_s32 = cella_ImageWidth_s32;
		cella_Seed_pu8 = (uint8_t *)malloc(cella_ImageWidth_s32 * sizeof(uint8_t));

		uint8_t condition_u8;
		for(index_s32 = CELLA_FIRST_COLUMN; index_s32 < cella_ImageWidth_s32; index_s32++)
		{
			condition_u8 = FALSE;
			condition_u8 |= 'l' == cella_Configuration_t.seedPosition_c && CELLA_FIRST_COLUMN == index_s32;
			condition_u8 |= 'r' == cella_Configuration_t.seedPosition_c && cella_ImageWidth_s32 - 1 == index_s32;
			condition_u8 |= 'l' != cella_Configuration_t.seedPosition_c &&
							'r' != cella_Configuration_t.seedPosition_c &&
							(((cella_ImageWidth_s32 + 1) / 2) == index_s32);
			
			if(TRUE == condition_u8)
				cella_Seed_pu8[index_s32] = BMP_COLOR_BLACK;
			else
				cella_Seed_pu8[index_s32] = BMP_COLOR_WHITE;
		}
	}
	
	printf("done\n");
}

static void cella_DetermineIterations(void)
{
	printf("Determining the number of iterations... ");

	if(TRUE == cella_Configuration_t.forceIterations_u8)
		cella_NumberOfIterations_u32 = cella_Configuration_t.forcedIterations_u32;
	else
		cella_NumberOfIterations_u32 = cella_ImageHeight_s32 - 1;

	printf("done\n");
}

static void cella_AllocatePixelData(void)
{
	printf("Allocating pixel data... ");

	cella_PixelData_ppu8 = (uint8_t **)malloc(cella_ImageHeight_s32 * sizeof(uint8_t *));

	for(int32_t row_s32 = CELLA_FIRST_ROW; row_s32 < cella_ImageHeight_s32; row_s32++)
		cella_PixelData_ppu8[row_s32] = (uint8_t *)malloc(cella_ImageWidth_s32 * sizeof(uint8_t));
	
	printf("done\n");
}

static void cella_InitializePixelData(void)
{
	printf("Initializing pixel data... ");

	int32_t row_s32, column_s32;

	for(row_s32 = CELLA_FIRST_ROW; row_s32 < cella_ImageHeight_s32; row_s32++)
		for(column_s32 = CELLA_FIRST_COLUMN; column_s32 < cella_ImageWidth_s32; column_s32++)
			cella_PixelData_ppu8[row_s32][column_s32] = BMP_COLOR_WHITE;
	
	int32_t seedCenter_s32;
	if(TRUE == cella_Configuration_t.forceSeed_u8)
		seedCenter_s32 = cella_ImageWidth_s32 / 2 - cella_SeedLength_s32 / 2;
	else
		seedCenter_s32 = 0;

	for(column_s32 = seedCenter_s32; column_s32 < seedCenter_s32 + cella_SeedLength_s32; column_s32++)
		cella_PixelData_ppu8[CELLA_FIRST_ROW][column_s32] = cella_Seed_pu8[column_s32 - seedCenter_s32];
	
	printf("done\n");
}

static void cella_DetermineOutputFileName(void)
{
	printf("Determining output file name... ");

	int32_t outputFileNameLength_s32;

	if(TRUE == cella_Configuration_t.forceOutputName_u8)
	{
		outputFileNameLength_s32 = strlen(cella_Configuration_t.forcedOutputName_pc);

		cella_OutputFileName_pc = (char *)malloc(outputFileNameLength_s32 * sizeof(char));

		for(int32_t index_s32 = 0; index_s32 < outputFileNameLength_s32; index_s32++)
			cella_OutputFileName_pc[index_s32] = cella_Configuration_t.forcedOutputName_pc[index_s32];
	}
	else
	{
		char buf_ac[32];
		time_t currentTime_t = time(0);
		struct tm* currentTimeInfo_st = localtime(&currentTime_t);
		sprintf(buf_ac, "output_%02d%02d%02d.bmp",
				currentTimeInfo_st -> tm_hour, currentTimeInfo_st -> tm_min, currentTimeInfo_st -> tm_sec);
		
		outputFileNameLength_s32 = strlen(buf_ac);

		cella_OutputFileName_pc = (char *)malloc(outputFileNameLength_s32 * sizeof(char));

		for(int32_t index_s32 = 0; index_s32 < outputFileNameLength_s32; index_s32++)
			cella_OutputFileName_pc[index_s32] = buf_ac[index_s32];
	}

	printf("done\n");
}

static void cella_OpenOutputFile(void)
{
	printf("Creating output file... ");

	cella_OutputFileDescriptor_fd = open(cella_OutputFileName_pc, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if(cella_OutputFileDescriptor_fd < 0)
		utils_ErrorMessage("creating output file");
	
	printf("done\n");
}
/**************** END INITIALIZE ****************/

/**************** BEGIN SIMULATION ****************/
void cella_RunSimulation(void)
{
	printf("Running simulation...\n");

	int32_t row_s32, column_s32;

	for(row_s32 = 1; row_s32 < cella_NumberOfIterations_u32; row_s32++)
	{
		if(row_s32 == cella_NumberOfIterations_u32 / 4)
			printf("\t25%%\n");
		else if(row_s32 == cella_NumberOfIterations_u32 / 2)
			printf("\t50%%\n");
		else if(row_s32 == 3 * cella_NumberOfIterations_u32 / 4)
			printf("\t75%%\n");
		
		for(column_s32 = CELLA_FIRST_COLUMN; column_s32 < cella_ImageWidth_s32; column_s32++)
			cella_PixelData_ppu8[row_s32][column_s32] = cella_ComputeCell(row_s32, column_s32);
	}
	printf("\t100%%\n");
}

static uint8_t cella_ComputeCell(int32_t row_s32, int32_t column_s32)
{
	uint8_t northWest_u8, north_u8, northEast_u8;

	if(CELLA_FIRST_COLUMN == column_s32)
		northWest_u8 = cella_PixelData_ppu8[row_s32 - 1][cella_ImageWidth_s32 - 1];
	else
		northWest_u8 = cella_PixelData_ppu8[row_s32 - 1][column_s32 - 1];
	
	north_u8 = cella_PixelData_ppu8[row_s32 - 1][column_s32];

	if(column_s32 == cella_ImageWidth_s32 - 1)
		northEast_u8 = cella_PixelData_ppu8[row_s32 - 1][CELLA_FIRST_COLUMN];
	else
		northEast_u8 = cella_PixelData_ppu8[row_s32 - 1][column_s32 + 1];
	
	return cella_ComputeRuleResult(northWest_u8, north_u8, northEast_u8);
}

static uint8_t cella_ComputeRuleResult(uint8_t northWest_u8, uint8_t north_u8, uint8_t northEast_u8)
{
	uint8_t index = 0;

	if(BMP_COLOR_WHITE != northEast_u8)	index |= 1;
	if(BMP_COLOR_WHITE != north_u8)		index |= 2;
	if(BMP_COLOR_WHITE != northWest_u8)	index |= 4;
	
	return cella_Rule_au8[7 - index];
}
/**************** END SIMULATION ****************/

/**************** BEGIN TERMINATE ****************/
void cella_Terminate(void)
{
	printf("Terminating program... \n");

	printf("\t");
	cella_CloseOutputFile();

	printf("\t");
	cella_FreeMemory();
}

static void cella_CloseOutputFile(void)
{
	printf("Closing output file... ");

	if(close(cella_OutputFileDescriptor_fd) < 0)
		utils_ErrorMessage("closing output file");
	
	printf("done\n");
}

static void cella_FreeMemory(void)
{
	printf("Freeing allocated memory... ");

	free(cella_OutputFileName_pc);
	free(cella_Seed_pu8);
	
	for(int32_t index_s32 = 0; index_s32 < cella_ImageHeight_s32; index_s32++)
		free(cella_PixelData_ppu8[index_s32]);
	free(cella_PixelData_ppu8);

	printf("done\n");
}
/**************** END TERMINATE ****************/