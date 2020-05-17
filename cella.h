#ifndef CELLA_H
#define CELLA_H

#define TRUE	1u
#define FALSE	0u

#define CELLA_DEFAULT_WIDTH		100
#define CELLA_DEFAULT_HEIGHT	100
#define CELLA_DEFAULT_RULE		30

#define CELLA_FIRST_ROW		0
#define CELLA_FIRST_COLUMN	0

typedef struct cella_configuration_st cella_configuration_t;

void cella_Initialize(int argc, char *argv[]);
static void cella_ParseParameters(int argc, char *argv[]);
static void cella_DetermineRule(void);
static void cella_DetermineImageDimensions(void);
static void cella_DetermineSeed(void);
static void cella_DetermineIterations(void);
static void cella_AllocatePixelData(void);
static void cella_InitializePixelData(void);
static void cella_DetermineOutputFileName(void);
static void cella_OpenOutputFile(void);

void cella_RunSimulation(void);
static uint8_t cella_ComputeCell(int32_t row_s32, int32_t column_s32);
static uint8_t cella_ComputeRuleResult(uint8_t northWest_u8, uint8_t north_u8, uint8_t northEast_u8);

void cella_Terminate(void);
static void cella_CloseOutputFile(void);
static void cella_FreeMemory(void);

#endif /* CELLA_H */