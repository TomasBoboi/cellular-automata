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
void cella_RunSimulation(void);
void cella_Terminate(void);

#endif /* CELLA_H */