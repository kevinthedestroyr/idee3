#include "idee3.h"

// CLI utility for reading and editing metadata of digital music files

int main(int argc, char **argv)
{
	unsigned idx, num_files;
	char c, ok;
	char *this_arg = NULL; 
	char *file_names[argc]; 			// array of file names
	FILE *music_file = NULL;
	data_t user_data;
	FIELD_NAME field_num = SIZE;
	FIELD_NAME field;

	// allocate memory for input file names
	if (!init_string_array(file_names, argc, MAX_NAME)) {
		exit(1);
	}

	// initialize user field inputs to NULL
	for (field = 0; field < SIZE; field++) {
		user_data.fields[field] = NULL;
	}

	// get command line options and arguments
	idx = 0;
	num_files = 0;
	while (--argc > 0) {
		if (**++argv == '-') { // option
			switch (*++*argv) {
				case 'r':
					user_data.rwflag = READ;
					break;
				case 'w':
					user_data.rwflag = WRITE;
					break;
				case 'h':
					print_usage();
					exit(0);
					break;
				default:
					printf("Error: unrecognized option: -%s\n", *argv);
					print_usage();
					exit(1);
					break;
			}
			// user has requested either read or write
			ok = get_user_fields(&argc, &argv, &user_data);
			if (!ok) {
				exit(1);
			}
		}
		else { // these should be the file names
			num_files++;
			strncpy(file_names[idx++], *argv, MAX_NAME);
		}
	}
	printf("input files:\n");
	for (idx = 0; idx < num_files; idx++) {
		printf("%s\n", file_names[idx]);
	}
	printf("write args:\n");
	for (field = 0; field < field_num; field++) {
		if (user_data.fields[field] != NULL) {
			printf("%s\n", user_data.fields[field]);
		}
	}
}

void print_usage() 
{
	printf("Usage: idee3 [OPTION]... FILE...\n"
		   "Read or edit the meta-data (e.g. id3v2 tag) of FILE(s)\n\n"
	   	   "  -h\t\t\tprint this message\n"
		   "  -w\t\t\twrite or edit meta-data of FILE\n\n");
}

char get_user_fields(int* argcount, char*** args, data_t* user_data) 
{
	char* this_arg;
	char done, retval;
	FIELD_NAME field;
	int data_flag;	// flag indicating that user field data should be next argument
					//	used for write mode only

	data_flag = 0;
	done = 0;
	retval = 1;
	while (!done && --*argcount) {
		this_arg = *((*args)+1);
		if (*this_arg == '-') {
			switch (*++this_arg) {
				case 'a':
					field = ARTIST;
					break;
				case 'l':
					field = ALBUM;
					break;
				case 'y':
					field = YEAR;
					break;
				case 'k':
					field = TRACK;
					break;
				case 't':
					field = TITLE;
					break;
				case 'w':
				case 'r':
					retval = 0;
					printf("Error: cannot combine read and write options, one thing at a time...\n");
					done = 1;
					return retval;
					break;
				default:
					retval = 0; // let caller know something went wrong
					printf("Error: Unexpected option used in combination with -r[ead]\n");
					done = 1;
					return retval;
					break;
			}
			if (!done) {
			   	++*args; // advance arg pointer if we used the arg
				user_data->fields[field] = malloc(sizeof(char) * MAX_FIELD);
				if (user_data->rwflag == WRITE) { // next argument should be input string
					if (--*argcount) {
						strncpy(user_data->fields[field], *++*args, MAX_FIELD);
					}
					else { // should have been an argument here
						printf("Error: no input found following write field option\n");
						retval = 0;
						return retval;
					}
				}
			}
		}
		else { // non '-' argument
			done = 1; 
			++*argcount;
		}
	}
	return retval;
}

char init_string_array(char* strarr[], int arr_size, int str_len) {
	int i;
	for (i = 0; i < arr_size; i++) {
		strarr[i] = malloc(sizeof(char) * str_len);
		if (strarr[i] == NULL) {
			printf("Error: out of memory\n");
			return 0;
		}
	}
	return 1;
}
