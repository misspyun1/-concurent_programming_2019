#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>

#define TUPLE_SIZE	(100)
#define KEY_SIZE	(10)

void split (int ) ;
void sort ( int) ;

const unsigned int MEM_SIZE = 2e9;

int compare(const void* p1, const void* p2) {
	return memcmp(p1, p2, KEY_SIZE);
}


int main(int argc, char* argv[]) {

	if (argc < 3) {
		printf("usage: ./run InputFile OutputFile\n");
		return 0;
	}

	int input_fd;
	input_fd = open(argv[1], O_RDONLY);
	if (input_fd == -1) {
		printf("error: open input file\n");
		return 0;
	}

	// split input file as two files.
	split(input_fd);

	// open output file.
	int output_fd;
	output_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (output_fd == -1) {
		printf("error: open output file\n");
		return 0;
	}
	// sort data and write to output file.
	sort(output_fd);

	// close input file.
	close(input_fd);
	// close output file.
	close(output_fd);


	return 0;
}

/* Splits the original file into two files */
void split ( int input_fd )
{
	int temp_fd;
	char *file_names[ ] ={"temp1.dat","temp2.dat"};


	size_t file_size = lseek(input_fd, 0, SEEK_END);
	size_t file_size_half[2];
	file_size_half[0]= (file_size /TUPLE_SIZE)/2 * TUPLE_SIZE;
	file_size_half[1] = file_size - file_size_half[0];

	size_t  offset = 0;

	for(int i  = 0;i<2;i++) {
		temp_fd = open(file_names[i], O_RDWR | O_CREAT | O_TRUNC, 0777);
		if (temp_fd == -1) {
			printf("error: open temp file\n");
			return;
		}

		char *buffer;
		buffer = (char *) malloc(sizeof(char) * MEM_SIZE);

		size_t ret = pread(input_fd, buffer, file_size_half[i], offset);
		offset = offset + ret;
		// sort.
		qsort(buffer, file_size_half[i] / TUPLE_SIZE, TUPLE_SIZE, compare);
		ret = pwrite(temp_fd, buffer, file_size_half[i], 0);
		if (ret < 0) {
			printf("error: write temp file\n");
			return;
		}

		// free buffer.
		free(buffer);

		close(temp_fd);
	}

}


/* Sorts the file */
void sort ( int output_fd)
{
	int temp_fd[2];
	char *file_names[ ] ={"temp1.dat","temp2.dat"};

	size_t  offset[2] = {0,0};
	size_t output_offset = 0;
	for(int i  = 0;i<2;i++) {
		temp_fd[i] = open(file_names[i], O_RDONLY);
		if (temp_fd[i] == -1) {
			printf("error: open temp file\n");
			return;
		}
	}
	size_t file_size = lseek(temp_fd[0], 0, SEEK_END);
	size_t file_size_half[2];
	file_size_half[0]= (file_size /TUPLE_SIZE)/2 * TUPLE_SIZE;
	file_size_half[1] = file_size - file_size_half[0];

	for(int i = 0;i<2;i++) {
		char *buffer;
		buffer = (char *) malloc(sizeof(char) * MEM_SIZE);

		size_t buffer_offset = 0;

		for (int tmp_file_num = 0; tmp_file_num < 2; tmp_file_num++) {
			size_t ret = pread(temp_fd[tmp_file_num], buffer + buffer_offset, file_size_half[i], offset[tmp_file_num]);
			if (ret < 0) {
				printf("error: read temp file\n");
			}
			offset[tmp_file_num] = offset[tmp_file_num] + ret;
			buffer_offset = buffer_offset + ret;
		}

		// sort.
		qsort(buffer, file_size_half[i] / TUPLE_SIZE, TUPLE_SIZE, compare);
		size_t ret = pwrite(output_fd, buffer, 2*file_size_half[i], output_offset);
		output_offset = output_offset + ret;
		if (ret < 0) {
			printf("error: write output file\n");
		}
		free(buffer);
	}

}




