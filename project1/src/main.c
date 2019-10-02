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

const unsigned int MEM_SIZE = 2e9;

void split (int);
void sort (int);

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
    
    // close input file.
	close(input_fd);

	// open output file.
	int output_fd;
	output_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (output_fd == -1) {
		printf("error: open output file\n");
		return 0;
	}
	// sort data and write to output file.
	sort(output_fd);


	// close output file.
	close(output_fd);


	return 0;
}

/* Splits the original file into two files */
void split ( int input_fd )
{
	int temp_fd;
	char *temp_file_names[ ] ={"temp1.dat","temp2.dat"};


	size_t file_size = lseek(input_fd, 0, SEEK_END);
    
	size_t  offset = 0;

	for(int i  = 0;i<2;i++) {
		temp_fd = open(temp_file_names[i], O_RDWR | O_CREAT | O_TRUNC, 0777);
		if (temp_fd == -1) {
			printf("error: open temp file\n");
			return;
		}

		char *buffer;
		buffer = (char *) malloc(sizeof(char) * MEM_SIZE);

		size_t ret = pread(input_fd, buffer, file_size/2, offset);
		offset = offset + ret;

		// sort.
		qsort(buffer, (file_size/2) / TUPLE_SIZE, TUPLE_SIZE, compare);
		ret = pwrite(temp_fd, buffer, file_size/2, 0);
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
void sort ( int output_fd )
{
    int temp_fd[2] ; // file descriptor of temp files
    size_t temp_file_offset[2] = {0,0};
    size_t output_offset = 0;
    char *temp_file_names[ ] ={"temp1.dat","temp2.dat",} ;
    size_t temp_file_size[2];

    size_t ret;
    unsigned int MEM_SIZE_HALF = 1e9-2e8;

    char *buffer1;
	buffer1 = (char *) malloc(sizeof(char) * MEM_SIZE_HALF/2);    
    char *buffer2;
	buffer2 = (char *) malloc(sizeof(char) * MEM_SIZE_HALF/2);
    char *output_buffer;
	output_buffer = (char *) malloc(sizeof(char) * MEM_SIZE_HALF);

    int  flag1, flag2;
    int buffer1_idx=0, buffer2_idx=0;
    size_t read_ret1, read_ret2;

    for (int i = 0 ; i <2 ; i++ ){
        temp_fd[i] = open(temp_file_names[i], O_RDONLY);
        if (temp_fd[i] == -1) {
            printf("error: open temp file\n");
            return;
        }

        temp_file_size[i] = lseek(temp_fd[i], 0, SEEK_END);
        if ( temp_file_size[i] == 0 ){
            printf("error : temp file empty\n");
            return;
        }
    }

    flag1 = flag2 = 0 ;
    read_ret1 = pread(temp_fd[0], buffer1, MEM_SIZE_HALF/2, temp_file_offset[0]);
    read_ret2 = pread(temp_fd[1], buffer2, MEM_SIZE_HALF/2, temp_file_offset[1]);
    temp_file_offset[0] += read_ret1;
    temp_file_offset[1] += read_ret2;

    while ( 1 )
    {
        if ( flag1 )
        {
            /* Make buffer1 empty to add new data */
            buffer1_idx = 0;
            read_ret1 = pread(temp_fd[0], buffer1, MEM_SIZE_HALF/2, temp_file_offset[0]);
            if ( read_ret1 == 0 )
            {   /* If first file ends then the whole content of second
                    file is written in the respective target file */
                ret = pwrite(output_fd, buffer2+buffer2_idx, read_ret2-buffer2_idx, output_offset);
                output_offset += ret;
                while (1){
                    /* Read temp2.dat to end, and write it to output_buffer*/
                    read_ret2 = pread ( temp_fd[1], output_buffer, MEM_SIZE_HALF, temp_file_offset[1] );
                    if(read_ret2 == 0){
                        break;
                    }
                    ret = pwrite(output_fd, output_buffer, read_ret2, output_offset);
                    output_offset += ret;
                    temp_file_offset[1] += read_ret2;
                }
                break ;
            }
            temp_file_offset[0] += read_ret1;
        }

        if ( flag2 )
        {
            /*Make buffer2 empty to add new data*/
            buffer2_idx = 0;
            read_ret2 = pread(temp_fd[1], buffer2, MEM_SIZE_HALF/2, temp_file_offset[1]);
            
            if ( read_ret2 == 0 )
            {   /* If first file ends then the whole content of second
                    file is written in the respective target file */
                ret = pwrite(output_fd, buffer1 + buffer1_idx, read_ret1-buffer1_idx, output_offset);
                output_offset += ret;
                while (1){
                    /* Read temp1.dat to end, and write it to output_buffer*/
                    read_ret1 = pread ( temp_fd[0], output_buffer, MEM_SIZE_HALF, temp_file_offset[0] );
                    if(read_ret1 == 0){
                        break;
                    }
                    ret = pwrite(output_fd, output_buffer, read_ret1, output_offset);
                    output_offset += ret;
                    temp_file_offset[0] += read_ret1;
                }
                break ;
            }
            temp_file_offset[1] += read_ret2;
        }
        
        size_t output_amount = 0;
        while(buffer1_idx < read_ret1 && buffer2_idx < read_ret2&&output_amount<MEM_SIZE_HALF){
            int cmp = compare(buffer1+buffer1_idx, buffer2+buffer2_idx);
            /* If cmp <0, it means buffer1 <buffer2 so copy buffer1 to output_buffer as much as TUPLE_SIZE
                if not, copy buffer2 to output_buffer */
            if ( cmp < 0)
            {
                flag2 = 0 ;
                flag1 = 1 ;
                memcpy(output_buffer+output_amount, buffer1+buffer1_idx, TUPLE_SIZE);
                buffer1_idx += TUPLE_SIZE;
            }
            else
            {
                flag1 = 0 ;
                flag2 = 1 ;
                memcpy(output_buffer+output_amount, buffer2+buffer2_idx, TUPLE_SIZE);
                buffer2_idx += TUPLE_SIZE;
            }
            output_amount += TUPLE_SIZE;
        }

        ret = pwrite(output_fd, output_buffer, output_amount, output_offset);
        if (ret < 0) {
            printf("error: write temp file\n");
            return;
        }
        output_offset += ret;

    }
    
    // free buffer.
	free(buffer1);
	free(buffer2);
    free(output_buffer);

	close(temp_fd[0]);
	close(temp_fd[1]);

}




