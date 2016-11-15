#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#include "matrix.h"


#define MAX_CMD_COUNT 50

/*protected functions*/
void load_matrix (Matrix_t* m, unsigned int* data);

/* 
 * PURPOSE: instantiates a new matrix with the passed name, rows, cols 
 * INPUTS: 
 *	name the name of the matrix limited to 25 characters 
 *  rows the number of rows the matrix
 *  cols the number of cols the matrix
 * RETURN:
 *  If no errors occurred during instantiation then true
 *  else false for an error in the process.
 *
 **/

bool create_matrix (Matrix_t** new_matrix, const char* name, const unsigned int rows,
						const unsigned int cols) {

	if((*new_matrix) != NULL)
	{
		//always gonna be NULL?
		printf("There's a matrix already here!\n");
		return false;
	}
	if(name == NULL || strcmp(name, "\n") == 0)
	{
		printf("There's no name for the matrix!\n");
		return false;
	}
	if(rows < 0 || rows > 4294967295)
	{
		//unsigned ints beyond allowed range
		printf("Invalid number of rows!\n");
		return false;	
	}
	if(cols < 0 || cols > 4294967295)
	{
		printf("Invalid number of columns!\n");
		return false;
	}

	*new_matrix = calloc(1,sizeof(Matrix_t));
	if (!(*new_matrix)) {
		printf("Failed to make new matrix\n");
		return false;
	}
	(*new_matrix)->data = calloc(rows * cols,sizeof(unsigned int));
	if (!(*new_matrix)->data) {
		printf("Failed to create space for new matrix\n");
		return false;
	}
	(*new_matrix)->rows = rows;
	(*new_matrix)->cols = cols;
	unsigned int len = strlen(name) + 1; 
	if (len > MATRIX_NAME_LEN) {
		printf("Matrix name is too long\n");
		return false;
	}
	strncpy((*new_matrix)->name,name,len);
	return true;

}

/*
Purpose: Frees the given memory that represents a matrix.
Inputs: A matrix of varying characteristics
Return: Nothing
*/
void destroy_matrix (Matrix_t** m) {

	if(*m == NULL)
	{
		printf("There's no matrix here!\n");
		return;
	}
	
	
	free((*m)->data);
	free(*m);
	*m = NULL;
}


/*
Purpose: Compares two matrices for equality across all characteristics 
Inputs: Two matrices of varying characteristics
return: 
	True if the matrices are identical
	False if they are not 
*/
bool equal_matrices (Matrix_t* a, Matrix_t* b) {

	if(a == NULL)
	{
		printf("The first matrix is NULL!\n");
		return false;
	}
	if(b == NULL)
	{
		printf("The second matrix is NULL!\n");
		return false;
	}
	if(!a->data)
	{
		printf("The first matrix is empty!\n");
		return false;
	}
	if(!b->data)
	{
		printf("The second matrix is empty!\n");
		return false;
	}

	//(error check done right here? -Daniel)	
	if (!a || !b || !a->data || !b->data) {
		return false;	
	}

	int result = memcmp(a->data,b->data, sizeof(unsigned int) * a->rows * a->cols);
	if (result == 0) {
		return true;
	}
	return false;
}

	
/*
Purpose: Takes a preexisting matrix and copies it into an empty matrix
Inputs: 
	src: a matrix that already exists
	dest: space for another matrix
return:
	true if the copying was successful
	false if the copy was not successful 
*/
bool duplicate_matrix (Matrix_t* src, Matrix_t* dest) {


	if (!src) {
		//this one has been done already!
		printf("Source matrix does not exist!\n");
		return false;
	}
	if(!dest)
	{
		//TODO (dan): call create_matrix?
		printf("No place to put the copied matrix!\n");
		return false;
	}
	if(!src->data)
	{
		printf("No data in the source matrix!\n");
		return false;
	}
	if(!dest->data)
	{
		//TODO (dan): call create_matrix?
		printf("No place to put the copied data!\n");
		return false;
	}	
	/*
	 * copy over data
	 */
	unsigned int bytesToCopy = sizeof(unsigned int) * src->rows * src->cols;
	memcpy(dest->data,src->data, bytesToCopy);	
	return equal_matrices (src,dest);
}

	
/*
Purpose: 
Inputs: 
return: 
*/
bool bitwise_shift_matrix (Matrix_t* a, char direction, unsigned int shift) {
	
	if (!a) {
		//This one's done already!
		printf("There is no matrix to do an operation on!");
		return false;
	}
	if(!a->data)
	{
		printf("There's no data in this matrix!\n");
		return false;
	}
	if(direction != 'l' && direction != 'r')
	{
		printf("Invalid shift direction!\n");
		return false;
	}
	if(shift < 0 || shift > 4294967295)
	{
		printf("Invalid amount to shift!\n");
		return false;
	}


	if (direction == 'l') {
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] << shift;
			}
		}

	}
	else {
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] >> shift;
			}
		}
	}
	
	return true;
}

	
/*
Purpose: Adds two matrices and puts the result into a new matrix added to the list
Inputs: Two preexisting matrices of varying characteristics. They must be identical in 
	rows and coumns to have a successful addition operation.
return: 
	True if the addition is successful.
	False if something went wrong.
*/
bool add_matrices (Matrix_t* a, Matrix_t* b, Matrix_t* c) {

	if(!a)
	{
		printf("The first matrix doesn't exist!\n");
		return false;
	}
	if(!b)
	{
		printf("The second matrix doesn't exist!\n");
		return false;
	}
	if(!c)
	{
		printf("The destination matrix doesn't exist!\n");
		return false;
	}
	if(!a->data)
	{
		printf("The first matrix doesn't have data!\n");
		return false;
	}
	if(!b->data)
	{
		printf("The second matrix doesn't have data!\n");
		return false;
	}	
	if(!c->data)
	{
		printf("The destination matrix doesn't have space to put data!\n");
		return false;
	}
	
	if (a->rows != b->rows && a->cols != b->cols) {
		return false;
	}

	for (int i = 0; i < a->rows; ++i) {
		for (int j = 0; j < b->cols; ++j) {
			c->data[i * a->cols +j] = a->data[i * a->cols + j] + b->data[i * a->cols + j];
		}
	}
	return true;
}

	
/*
Purpose:Prints out a matrix to the screen in a way such that it looks like a matrix.  
Inputs: A preexisting matrix of varying characteristics.
return: Nothing.
*/
void display_matrix (Matrix_t* m) {
	
	if(!m)
	{
		printf("Matrix doesn't exist!\n");
		return ;
	}
	if(!m->data)
	{
		printf("Matrix doesn't have any data!\n");
		return ;
	}

	printf("\nMatrix Contents (%s):\n", m->name);
	printf("DIM = (%u,%u)\n", m->rows, m->cols);
	for (int i = 0; i < m->rows; ++i) {
		for (int j = 0; j < m->cols; ++j) {
			printf("%u ", m->data[i * m->cols + j]);
		}
		printf("\n");
	}
	printf("\n");

}

	
/*
Purpose: Implements the option to read matrix data off of a file. 
Inputs: 
	A file that exists on the system. The path can be either absolute or relative.
	An empty matrix that exists and has valid characteristics.
return: 
	True if the file was read successfully and the matrix was created successfully.
	False if something went wrong along the way.
*/
bool read_matrix (const char* matrix_input_filename, Matrix_t** m) {
	
	if(!m)
	{
		printf("No matrix to store read data!\n");
		return false;
	}
	if(!(*m)->data)
	{
		printf("Matrix does not have space to store read data!\n");
		return false;
	}
	


	int fd = open(matrix_input_filename,O_RDONLY);
	if (fd < 0) {
		printf("FAILED TO OPEN FOR READING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}

	/*read the wrote dimensions and name length*/
	unsigned int name_len = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	
	if (read(fd,&name_len,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READING FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	char name_buffer[50];
	if (read (fd,name_buffer,sizeof(char) * name_len) != sizeof(char) * name_len) {
		printf("FAILED TO READ MATRIX NAME\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (read (fd,&rows, sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX ROW SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	if (read(fd,&cols,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX COLUMN SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	unsigned int numberOfDataBytes = rows * cols * sizeof(unsigned int);
	unsigned int *data = calloc(rows * cols, sizeof(unsigned int));
	if (read(fd,data,numberOfDataBytes) != numberOfDataBytes) {
		printf("FAILED TO READ MATRIX DATA\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (!create_matrix(m,name_buffer,rows,cols)) {
		return false;
	}

	load_matrix(*m,data);
	free(data);
	if (close(fd)) {
		return false;

	}
	return true;
}

	
/*
Purpose: writes matrix data out to a file.  
Inputs: 
	Destination filepath. Can be relative or absolute.
	The matrix to be written to the file.
return: 
	True if the matrix was successfully written to the file
	False if something went wrong along the way
*/
bool write_matrix (const char* matrix_output_filename, Matrix_t* m) {
	
	if(!m)
	{
		printf("No matrix to store read data!\n");
		return false;
	}
	if(!m->data)
	{
		printf("Matrix does not have space to store read data!\n");
		return false;
	}

	int fd = open (matrix_output_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
	/* ERROR HANDLING USING errorno*/
	if (fd < 0) {
		printf("FAILED TO CREATE/OPEN FILE FOR WRITING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXISTS\n");
		}
		return false;
	}
	/* Calculate the needed buffer for our matrix */
	unsigned int name_len = strlen(m->name) + 1;
	unsigned int numberOfBytes = sizeof(unsigned int) + (sizeof(unsigned int)  * 2) + name_len + sizeof(unsigned int) * m->rows * m->cols + 1;
	/* Allocate the output_buffer in bytes
	 * IMPORTANT TO UNDERSTAND THIS WAY OF MOVING MEMORY
	 */
	unsigned char* output_buffer = calloc(numberOfBytes,sizeof(unsigned char));
	unsigned int offset = 0;
	memcpy(&output_buffer[offset], &name_len, sizeof(unsigned int)); // IMPORTANT C FUNCTION TO KNOW
	//TODO: (dan) discect this code
	offset += sizeof(unsigned int);	
	memcpy(&output_buffer[offset], m->name,name_len);
	offset += name_len;
	memcpy(&output_buffer[offset],&m->rows,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&output_buffer[offset],&m->cols,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy (&output_buffer[offset],m->data,m->rows * m->cols * sizeof(unsigned int));
	offset += (m->rows * m->cols * sizeof(unsigned int));
	output_buffer[numberOfBytes - 1] = EOF;

	if (write(fd,output_buffer,numberOfBytes) != numberOfBytes) {
		printf("FAILED TO WRITE MATRIX TO FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	
	if (close(fd)) {
		return false;
	}
	free(output_buffer);

	return true;
}

	
/*
Purpose: Creates a matrix filled with random numbers between a given range 
Inputs: 
	An empty matrix to be filled with random numbers
	A lower bound for the random number generated
	An upper bound for the random number generated
return: 
	True if the matrix was filled successfully
	False if there was a bad parameter sent to the function
*/
bool random_matrix(Matrix_t* m, unsigned int start_range, unsigned int end_range) {
	
	if(!m)
	{
		printf("Matrix doesn't exist!\n");
		return false;
	}
	if(!m->data)
	{
		printf("Matrix doesn't have space to hold data!\n");
		return false;
	}
	if(start_range < 0 || start_range > 4294967295)
	{
		printf("Lower bound of the range is invalid!\n");
		return false;
	}
	if(end_range < 0 || end_range > 4294967295)
	{
		printf("Upper bound of the range is invalid!\n");
		return false;
	}

	for (unsigned int i = 0; i < m->rows; ++i) {
		for (unsigned int j = 0; j < m->cols; ++j) {
			m->data[i * m->cols + j] = rand() % (end_range + 1 - start_range) + start_range;
		}
	}
	return true;
}

/*Protected Functions in C*/

	
/*
Purpose: Takes a matrix and fills it with the new data. 
Inputs: 
	An empty matrix of varying characteristics
	A set of data that will be loaded into the matrix
return: Nothing
*/
void load_matrix (Matrix_t* m, unsigned int* data) {
	
	if(!m)
	{
		printf("Matrix doesn't exist!\n");
		return;
	}
	if(!m->data)
	{
		printf("Matrix has no space to put data!\n");
		return;
	}
	if(!data)
	{
		printf("There's no data to load!\n");
		return;
	}
	
	memcpy(m->data,data,m->rows * m->cols * sizeof(unsigned int));
}

	
/*
Purpose: Takes a matrix that was created and adds it to the list of preexisting matrices.
	 If there is a preexisting matrix in the spot that the new matrix should go, the old matrix
	 is overriden.
Inputs: 
	A list of preexisting matrices.
	A new matrix to be added to the list.
	The number of matrices that already exist.
return: 
	The position that the new matrix was inserted.
*/
unsigned int add_matrix_to_array (Matrix_t** mats, Matrix_t* new_matrix, unsigned int num_mats) {
	
	if(!mats)
	{
		printf("There is no list of matrices!\n");
		return -1;
	}
	if(num_mats > 10)
	{
		printf("There's too many matrices in the list!\n");
		return -1;
	}
	if(num_mats < 0)
	{
		printf("There's something wrong with the number of matrices!\n");
		return -1;
	}
	if(!new_matrix)
	{
		printf("No matrix to be added!\n");
		return -1;
	}
	if(!new_matrix->data)
	{
		printf("The new matrix doesn't have any data!\n");
		return -1;
	}


	static long int current_position = 0;
	const long int pos = current_position % num_mats;
	if ( mats[pos] ) {
		destroy_matrix(&mats[pos]);
	} 
	mats[pos] = new_matrix;
	current_position++;
	return pos;
}
