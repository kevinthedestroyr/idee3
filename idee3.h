#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define BUF_SIZE	100		// size of input buffer
#define MAX_NAME	100		// max allowable length of input file names
#define NUM_FIELDS	5
#define MAX_FIELD	1024	// 1k should be more than enough for text fields

#define READ 		0
#define WRITE		1

// define field string array positions, used in data_t below
typedef enum {
	ARTIST,
	ALBUM,
	TITLE,
	TRACK,
	YEAR, 
	SIZE
			} FIELD_NAME;

typedef struct track_data{
	int rwflag; 				// flags to indicate which fields are populated, and r/w mode
	char* fields[NUM_FIELDS];	// string array of user data for writing to fields
} data_t;

void print_usage();
char get_user_fields(int* argcount, char*** args, data_t* user_data);
char init_string_array(char* strarr[], int arr_size, int str_len); 
