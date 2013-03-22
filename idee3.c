#include "idee3.h"

// CLI utility for reading and editing metadata of digital music files

int main(int argc, char **argv)
{
	unsigned idx, num_files;
	int errorno, i;
	char c, ok;
	char *this_arg = NULL; 
	char *file_names[argc]; 			// array of file names
	char *buffer;
	FILE *fp = NULL;
	size_t result;
	data_t user_data;
	FIELD_NAME field_num = SIZE;
	FIELD_NAME field;
	header_info_t header;

	// allocate memory for input file names
	if (!init_string_array(file_names, argc, MAX_NAME)) {
		exit(1);
	}

	// initialize user field inputs to NULL
	for (field = 0; field < SIZE; field++) {
		user_data.fields[field] = NULL;
	}

	user_data.new_file = NULL; // initialize to NULL

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
	// now loop through each input file and do work
	buffer = malloc(sizeof(char) * BUF_SIZE);
	if (buffer == NULL) {
		printf("Error: out of memory\n");
		exit(1);
	}
	int file_index;
	for (file_index = 0; file_index < num_files; file_index++) {
		fp = fopen(file_names[file_index], "r");
		// if write mode, open up a file for writing to
		if (user_data.rwflag == WRITE) {
			char new_file_name[MAX_NAME + 4];
			strcpy(new_file_name, file_names[file_index]);
			strcat(new_file_name, ".new");
			printf("new file name: %s\n", new_file_name);
			user_data.new_file = fopen(new_file_name, "w");
			if (user_data.new_file == NULL) {
				printf("Error: could not create file %s for writing\n",
						new_file_name);
			}
		}
		if (fp == NULL) {
			printf("Error: unable to open file: %s\n", file_names[file_index]);
			exit(1);
		}
		ok = find_id3_start(&fp, &header, &buffer, &idx, BUF_SIZE);
		if (!ok) {
			fclose(fp);
			exit(1);
		}
		char more_tags = 1;
		char* tag = malloc(sizeof(char)*TAG_SIZE + 1);
		tag[TAG_SIZE] = '\0'; // null terminate the tag name buffer
		while (more_tags) {
			// refill the buffer
			if (idx >= BUF_SIZE) {
				idx = 0;
				size_t count = fread(buffer, sizeof(char), BUF_SIZE, fp);
				if (count < sizeof(char)*BUF_SIZE) {
					// couldn't fill buf
					if (feof(fp)) {
						more_tags = 0;
					}
					else {
						printf("Error: file read error: %d\n", errorno = ferror(fp));
						fclose(fp);
						exit(errorno);
					}
				}
			}
			memcpy(tag, &buffer[idx], sizeof(char)*TAG_SIZE);
			for (i = 0; i < TAG_SIZE; i++) {
				if (!isupper(tag[i]) && !isdigit(tag[i])) {
					// this is not a legit tag
					more_tags = 0;
					break;
				}
			}
			if (more_tags == 0) break;
			idx += TAG_SIZE; // advance our buffer index
			int fieldsize = 0;
			int cur_pos;
			for (cur_pos = idx; idx - cur_pos  < 4 && idx < BUF_SIZE; idx++) {
				fieldsize += (buffer[idx] << 8*(3 - idx + cur_pos));
			}
			if (idx + fieldsize + 2 + TAG_SIZE >= BUF_SIZE) { // two is for the two flag bytes
				fseek(fp, cur_pos - idx, SEEK_CUR); // put file point to start of this tag
				continue;
			}
			// skip flag bytes
			idx += 2;
			if (fieldsize <= MAX_FIELD) {
				int fieldint = (int) field_from_tag(tag);
				if (fieldint != -1 && user_data.fields[fieldint] != NULL) {
					memcpy(user_data.fields[fieldint], &buffer[idx], fieldsize);
					adjust_spacing(user_data.fields[fieldint], fieldsize);
				}
			}
			idx += fieldsize;

		}
		fclose(fp);
		if (user_data.new_file != NULL) {
			fclose(user_data.new_file);
		}
		for (field = 0; field < SIZE; field++) {
			if (user_data.fields[field] != NULL) {
				printf("%s\n", user_data.fields[field]);
			}
		}
	}


	// free all malloced memory
	for (field = 0; field < SIZE; field++) {
		if (user_data.fields[field] != NULL) {
			free(user_data.fields[field]);
		}
	}
	for (idx = 0; idx <  num_files; idx++) {
		if (file_names[idx] != NULL) {
			free(file_names[idx]);
		}
	}
}

// Search file given by fp for start of ID3 tag and return with
// file pointer at first byte after header (i.e. first field)
char find_id3_start(FILE** fp, header_info_t* header, char** buf, int* buf_idx, int bufsz) {
	size_t result;
	char* subbuf;
	int i;
	
	subbuf = malloc(sizeof(char) * bufsz);
	if (subbuf == NULL) {
		printf("Error: out of memory\n");
		return 0;
	}

	result = fread(*buf, sizeof(char), bufsz, *fp);
	if (result != bufsz*sizeof(char)) {
		printf("Error: error occurred while reading from file\n");
		return 0;
	}
	*buf_idx = 0;
	// check if tag is first thing in file (it often is)
	if (strncmp(*buf, "ID3", 3) == 0) {
		// found it - collect info
		*buf_idx += 3; // advance buffer passed "ID3"
		memcpy(subbuf, &(*buf)[*buf_idx], NUM_VERS_BYTES);
		header->version = 0;
		for (i = 0; i < NUM_VERS_BYTES; i++) {
			header->version |= subbuf[i] << 8*i;
		}
		(*buf_idx) += NUM_VERS_BYTES;
		header->flags = (*buf)[*buf_idx]; // 1 flags byte
		(*buf_idx) += NUM_FLAG_BYTES;
		memcpy(subbuf, &(*buf)[*buf_idx], NUM_SIZE_BYTES);
		header->size = 0;
		for (i = 0; i < NUM_SIZE_BYTES; i++) {
			header->size |= subbuf[i] << 8*(NUM_SIZE_BYTES - i - 1);
		}
		*buf_idx += NUM_SIZE_BYTES;
	}
	free(subbuf);
	return 1;
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

FIELD_NAME field_from_tag(char* tag_str) {
	FIELD_NAME result;
	if (strcmp(tag_str, "TIT2") == 0) {
		result = TITLE;
		return result;
	}
	if (strcmp(tag_str, "TALB") == 0) {
		result = ALBUM;
		return result;
	}
	if (strcmp(tag_str, "TPE1") == 0) {
		result = ARTIST;
		return result;
	}
	if (strcmp(tag_str, "TRCK") == 0) {
		result = TRACK;
		return result;
	}
	if (strcmp(tag_str, "TYER") == 0) {
		result = YEAR;
		return result;
	}
}

// undoes tags spaced out with nulls between each character,
// this pattern is indicated by the flag 0x01fffe at the start
// of the string.
void adjust_spacing(char *field_string, int fieldsize) {
	int i = 0;
	char spaced = 1;
	if (!(field_string[i] == '\001') ||
			!(field_string[++i] == '\377') ||
			!(field_string[++i] == '\376')) {
		// nothing to do here
		spaced = 0;
	}
	i++; // go to first char of actual string
	int k = 0;
	while (i < fieldsize) {
		field_string[k++] = field_string[i++];
		if (spaced) {
			i++; // skip null space
		}
	}
	field_string[k] = '\0'; // null terminate
}
