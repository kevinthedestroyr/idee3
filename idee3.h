#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define BUF_SIZE		1024	// size of input buffer
#define MAX_NAME		100		// max allowable length of input file names
#define NUM_FIELDS		5
#define MAX_FIELD		1024	// 1k should be more than enough for text fields
#define TAG_SIZE		4		// standard length of field tag in bytes

#define STD_HEAD_SIZE	10
#define NUM_FLAG_BYTES	1
#define NUM_VERS_BYTES	2
#define NUM_SIZE_BYTES	4

#define READ 			0
#define WRITE			1

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
	FILE* new_file;				// if in write mode, pointer to edited file
} data_t;

typedef struct {
	int version;
	int flags;
	int size;
} header_info_t;

void print_usage();
char get_user_fields(int* argcount, char*** args, data_t* user_data);
char init_string_array(char* strarr[], int arr_size, int str_len); 
char find_id3_start(FILE**, header_info_t*, char** buffer, int bufsize);
char parse_tag(char* tag, int tag_size, data_t* user_data);
