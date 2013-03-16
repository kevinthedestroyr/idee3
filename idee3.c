#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// CLI utility for reading and editing metadata of digital music files

#define BUF_SIZE	100		// size of input buffer
#define READ		0		// flag indicating tags to be read only
#define EDIT		1		// flag indicating tags to be edited
#define MAX_NAME	100		// max allowable length of input file names

// define field flags
#define ARTIST		1
#define ALBUM		2
#define TITLE		4	
#define TRACK		8
#define YEAR		16

typedef struct track_data{
	int flags; 				// flags to indicate which fields are populated
	char *artist;
	char *album;
	char *tracknum;
	char *year;
	char *title;
} data_t;

void print_usage();
void get_read_fields(int* argcount, char*** args, char* field_flags);
void get_write_fields(int* argcount, char*** args, data_t* user_data, char* field_flags); 

int main(int argc, char **argv)
{
	unsigned idx, num_files;
	char c, rwflag, *this_arg = NULL; 
	char *file_names[argc]; 			// array of file names
	char *field_flags = NULL; 			// flags indicating data CLI data given by user
	FILE *music_file = NULL;
	data_t user_data;
	
	//initialize music_files array
	for (idx = 0; idx < argc; idx++) {
		file_names[idx] = (char*)malloc(sizeof(char) * MAX_NAME);
	}

	// get command line options and arguments
	idx = 0;
	num_files = 0;
	field_flags = (char*)malloc(sizeof(char));
	*field_flags = 0;
	while (--argc > 0) {
		if (**++argv == '-') { // option
			switch (*++*argv) {
				case 'r':
					rwflag = READ;
					get_read_fields(&argc, &argv, field_flags);
					if (field_flags == NULL) {
						exit(1);
					}
					break;
				case 'w':
					rwflag = EDIT;
					get_write_fields(&argc, &argv, &user_data, field_flags);
					if (field_flags == NULL) {
						exit(1);
					}
					break;
				case 'h':
					print_usage();
					break;
				default:
					print_usage();
					exit(1);
					break;
			}
		}
		else { // these should be the file names
			num_files++;
			strncpy(file_names[idx++], *argv, MAX_NAME);
		}
	}
	for (idx = 0; idx < num_files; idx++) {
		printf("%s\n", file_names[idx]);
	}
}

void print_usage() {
	printf("Usage: idee3 [OPTION]... FILE...\n"
		   "Read or edit the meta-data (e.g. id3v2 tag) of FILE(s)\n\n"
	   	   "  -h\t\t\tprint this message\n"
		   "  -w\t\t\twrite or edit meta-data of FILE\n\n");
}

void get_read_fields(int* argcount, char*** args, char* field_flags) {
	char* this_arg;
	char done = 0;
	while (!done && --*argcount) {
		this_arg = *((*args)+1);
		if (*this_arg == '-') {
			switch (*++this_arg) {
				case 'a':
					*field_flags |= ARTIST;
					break;
				case 'l':
					*field_flags |= ALBUM;
					break;
				case 'y':
					printf("you want me to fetch the year\n");
					*field_flags |= YEAR;
					break;
				case 'k':
					*field_flags |= TRACK;
					break;
				case 't':
					*field_flags |= TITLE;
					break;
				default:
					field_flags = NULL;
					printf("Error: Unexpected option used in combination with -r[ead]\n");
					done = 1;
			}
			if (!done) ++*args; // only advance arg pointer if we used the arg
		}
		else {
			done = 1; 
			++*argcount;
		}
	}
}

void get_write_fields(int* argcount, char*** args, data_t* user_data, char* flags) {
	// fill this in
}
