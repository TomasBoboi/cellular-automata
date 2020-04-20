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

#define SEED_ARG		1
#define ITERATIONS_ARG	2
#define RULE_ARG		3
#define OUTPUT_FILE_ARG	4

void error_message(char *err);

void grab_rule(char *rule_s);
void calculate_image_dimensions(char *seed_s, char *iterations_s);
void grab_seed(char *seed_s);
void allocate_pixel_data();
void initialize_pixel_data();
void open_output_file(char *file_name);
void initialize(int argc, char *argv[]);

void run_simulation();
uint8_t compute_cell(int32_t row, int32_t column);
uint8_t get_rule(uint8_t north_west, uint8_t north, uint8_t north_east);

void close_output_file();
void terminate();

int32_t width, height;
uint8_t **pixel_data;
uint8_t *seed;
uint8_t rule[8];
int output_file;

int main(int argc, char *argv[])
{	
	initialize(argc, argv);

	run_simulation();

	writeImage(output_file, pixel_data, width, height);

	terminate();
	
	return 0;
}

void error_message(char *err)
{
	printf("Error: %s\n", err);
	exit(EXIT_FAILURE);
}
/* BEGIN INITIALIZE */
void initialize(int argc, char *argv[])
{
	grab_rule(argv[RULE_ARG]);
	calculate_image_dimensions(argv[SEED_ARG], argv[ITERATIONS_ARG]);
	grab_seed(argv[SEED_ARG]);
	allocate_pixel_data();
	initialize_pixel_data();

	if(OUTPUT_FILE_ARG >= argc)
	{
		char temp_file_name[32];
		strcpy(temp_file_name, "output_");

		time_t current_time = time(0);
		struct tm* current_time_info = localtime(&current_time);
		char buf[16];
		sprintf(buf, "%02d%02d%02d", current_time_info -> tm_hour, current_time_info -> tm_min, current_time_info -> tm_sec);
		strcat(temp_file_name, buf);

		strcat(temp_file_name, ".bmp");
		open_output_file(temp_file_name);
	}
	else
		open_output_file(argv[OUTPUT_FILE_ARG]);
}

void grab_rule(char *rule_s)
{
	printf("Grabbing rule... ");

	uint8_t rule_u8 = (uint8_t)atoi(rule_s);

	for(int32_t count = 7; count >= 0; count--)
	{
		rule[count] = (rule_u8 % 2 != 0) ? COLOR_BLACK : COLOR_WHITE;
		rule_u8 /= 2;
	}

	printf("done\n");
}

void calculate_image_dimensions(char *seed_s, char *iterations_s)
{
	printf("Calculating image dimensions... ");

	width = strlen(seed_s);
	height = atoi(iterations_s) + 1;

	printf("done\n");
}

void grab_seed(char *seed_s)
{
	printf("Grabbing seed... ");

	if(strlen(seed_s) != width)
		error_message("length of seed is incorrect");
	
	seed = (uint8_t*)malloc(width * sizeof(uint8_t));
	for(uint8_t column = 0; column < width; column++)
		seed[column] = ((seed_s[column] == '0') ? COLOR_WHITE : COLOR_BLACK);
	
	printf("done\n");
}

void allocate_pixel_data()
{
	printf("Allocating pixel data... ");

	pixel_data = (uint8_t**)malloc(height * sizeof(uint8_t*));

	for(int32_t row = 0; row < height; row++)
		pixel_data[row] = (uint8_t*)malloc(width * sizeof(uint8_t));
	
	printf("done\n");
}

void initialize_pixel_data()
{
	int32_t row, column;

	for(row = 0; row < height; row++)
		for(column = 0; column < width; column++)
			pixel_data[row][column] = COLOR_WHITE;
	
	for(column = 0; column < width; column++)
		pixel_data[0][column] = seed[column];
}

void open_output_file(char *file_name)
{
	printf("Creating output file... ");

	if((output_file = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO)) < 0)
		error_message("creating output file");
	
	printf("done\n");
}
/* END INITIALIZE */

/* BEGIN SIMULATION */
void run_simulation()
{
	int32_t row, column;

	for(row = 1; row < height; row++)
		for(column = 0; column < width; column++)
			pixel_data[row][column] = compute_cell(row, column);
}

uint8_t compute_cell(int32_t row, int32_t column)
{
	if(0 == column)
		return get_rule(pixel_data[row - 1][width - 1], pixel_data[row - 1][column], pixel_data[row - 1][column + 1]);
	else if(column == width - 1)
		return get_rule(pixel_data[row - 1][column - 1], pixel_data[row - 1][column], pixel_data[row - 1][0]);
	else
		return get_rule(pixel_data[row - 1][column - 1], pixel_data[row - 1][column], pixel_data[row - 1][column + 1]);
}

uint8_t get_rule(uint8_t north_west, uint8_t north, uint8_t north_east)
{
	uint8_t index = 0;

	if(COLOR_WHITE != north_east)
		index |= 1;
	if(COLOR_WHITE != north)
		index |= 2;
	if(COLOR_WHITE != north_west)
		index |= 4;
	
	return rule[7 - index];
}
/* END SIMULATION */

/* BEGIN TERMINATE */
void terminate()
{
	close_output_file();
	free(pixel_data);
	free(seed);
}

void close_output_file()
{
	printf("Closing output file... ");

	if(close(output_file) < 0)
		error_message("closing output file");
	
	printf("done\n");
}
/* END TERMINATE */