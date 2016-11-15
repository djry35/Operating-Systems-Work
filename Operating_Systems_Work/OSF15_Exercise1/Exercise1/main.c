#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include<readline/readline.h>

#include "command.h"
#include "matrix.h"
void run_commands (Commands_t* cmd, Matrix_t** mats, unsigned int num_mats);
unsigned int find_matrix_given_name (Matrix_t** mats, unsigned int num_mats, 
			const char* target);

void destroy_remaining_heap_allocations(Matrix_t **mats, unsigned int num_mats);

/*
Purpose: The central HQ for all the functions to be ran. The flow of the program is as follows:
	 create dummy matrices with empty data -> read user input commands -> parse commands ->
	 process commands -> empty command array -> repeat above until exit command -> destroy
	 matrices
Inputs: None
Return: 
	-1 For failed process
	0 for successful termination
*/
int main (int argc, char **argv) {
	srand(time(NULL));		
	char *line = NULL;
	Commands_t* cmd;

	Matrix_t *mats[10];
	memset(&mats,0, sizeof(Matrix_t*) * 10); // IMPORTANT C FUNCTION TO LEARN

	Matrix_t *temp = NULL;
	if(!create_matrix (&temp,"temp_mat", 5, 5))
	{
		printf("Something went wrong with initializing matrix. Terminating program...\n");
		return -1;
	}

	if(add_matrix_to_array(mats,temp, 10) == -1)
	{
		printf("Something went wrong with adding a matrix to the array. Terminating program...\n");
		destroy_remaining_heap_allocations(mats,10);
		return -1;
	}

	int mat_idx = find_matrix_given_name(mats,10,"temp_mat");

	if (mat_idx < 0) {
		perror("PROGRAM FAILED TO INIT\n");
		return -1;
	}
	random_matrix(mats[mat_idx], 10, 15);
	
	if(!write_matrix("temp_mat", mats[mat_idx]))
	{ 
		printf("Matrix failed to write to the file. Program will continue, but files might not work.\n");
	}

	line = readline("> ");
	while (strncmp(line,"exit", strlen("exit")  + 1) != 0) {
		
		if (!parse_user_input(line,&cmd)) {
			printf("Failed at parsing command\n\n");
		}
		
		if (cmd->num_cmds > 1) {	
			run_commands(cmd,mats,10);
		}
		if (line) {
			free(line);
		}
		destroy_commands(&cmd);
		line = readline("> ");
	}
	free(line);
	destroy_remaining_heap_allocations(mats,10);
	return 0;	
}

/*
Purpose: After user commands have been parsed to an array, this function will interpret and process
	 the command words from that array. 
Input:
	An array of parsed commands
	An array of matrices 
	The number of matrices that exist
Return: None
*/
void run_commands (Commands_t* cmd, Matrix_t** mats, unsigned int num_mats) {

	if(!cmd || !(cmd)->cmds)
	{
		printf("There are no commands to process!\n");
		return;
	}
	if(!mats || !(*mats))
	{
		printf("There are no matrices!\n");
		return;
	}
	if(num_mats < 0 || num_mats > 4294967295)
	{
		printf("There's something wrong with the number of matrices!\n");
		return;
	}

	/*Parsing and calling of commands*/
	if (strncmp(cmd->cmds[0],"display",strlen("display") + 1) == 0
		&& cmd->num_cmds == 2) {
			/*find the requested matrix*/
			int idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
			if (idx >= 0) {
				display_matrix (mats[idx]);
			}
			else {
				printf("Matrix (%s) doesn't exist\n", cmd->cmds[1]);
				return;
			}
	}
	else if (strncmp(cmd->cmds[0],"add",strlen("add") + 1) == 0
		&& cmd->num_cmds == 4) {
			int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
			int mat2_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[2]);
			if (mat1_idx >= 0 && mat2_idx >= 0) {
				Matrix_t* c = NULL;
				if( !create_matrix (&c,cmd->cmds[3], mats[mat1_idx]->rows, 
						mats[mat1_idx]->cols)) {
					printf("Failure to create the result Matrix (%s)\n", cmd->cmds[3]);
					return;
				}
			
				if(!add_matrix_to_array(mats,c, num_mats))
				{
					printf("Failure to add matrix (%s) to the array\n", cmd->cmds[3]);
					return;
				}

				if (! add_matrices(mats[mat1_idx], mats[mat2_idx],c) ) {
					printf("Failure to add %s with %s into %s\n", mats[mat1_idx]->name, mats[mat2_idx]->name,c->name);
					return;	
				}
			}
	}
	else if (strncmp(cmd->cmds[0],"duplicate",strlen("duplicate") + 1) == 0
		&& cmd->num_cmds == 3 && strlen(cmd->cmds[1]) + 1 <= MATRIX_NAME_LEN) {
		int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
		if (mat1_idx >= 0 ) {
				Matrix_t* dup_mat = NULL;
				if( !create_matrix (&dup_mat,cmd->cmds[2], mats[mat1_idx]->rows, 
						mats[mat1_idx]->cols)) {
					return;
				}
				
				if(!duplicate_matrix (mats[mat1_idx], dup_mat))
				{
					printf("Failure to duplicate matrix %d.\n", mat1_idx);
					return;
				}
				
				if(!add_matrix_to_array(mats,dup_mat,num_mats))
				{
					printf("Failure to add the duplicate of %d to the array of matrices.\n", mat1_idx);
					return;
				}
				
				printf ("Duplication of %s into %s finished\n", mats[mat1_idx]->name, cmd->cmds[2]);
		}
		else {
			printf("Duplication Failed\n");
			return;
		}
	}
	else if (strncmp(cmd->cmds[0],"equal",strlen("equal") + 1) == 0
		&& cmd->num_cmds == 2) {
			int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
			int mat2_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[2]);
			if (mat1_idx >= 0 && mat2_idx >= 0) {
				if ( equal_matrices(mats[mat1_idx],mats[mat2_idx]) ) {
					printf("SAME DATA IN BOTH\n");
				}
				else {
					printf("DIFFERENT DATA IN BOTH\n");
				}
			}
			else {
				printf("Equal Failed\n");
				return;
			}
	}
	else if (strncmp(cmd->cmds[0],"shift",strlen("shift") + 1) == 0
		&& cmd->num_cmds == 4) {
		int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
		const int shift_value = atoi(cmd->cmds[3]);
		if (mat1_idx >= 0 ) {
			
			if(!bitwise_shift_matrix(mats[mat1_idx],cmd->cmds[2][0], shift_value))
			{
				printf("Matrix %d could not be shifted.\n", mat1_idx);
				return;
			}
			printf("Matrix (%s) has been shifted by %d\n", mats[mat1_idx]->name, shift_value);
		}
		else {
			printf("Matrix shift failed\n");
			return;
		}

	}
	else if (strncmp(cmd->cmds[0],"read",strlen("read") + 1) == 0
		&& cmd->num_cmds == 2) {
		Matrix_t* new_matrix = NULL;
		if(! read_matrix(cmd->cmds[1],&new_matrix)) {
			printf("Read Failed\n");
			return;
		}	
		
		if(!add_matrix_to_array(mats,new_matrix, num_mats))
		{
			printf("Matrix (%s) could not be added to the list of matrices.\n", cmd->cmds[1]);
			return;
		}

		printf("Matrix (%s) is read from the filesystem\n", cmd->cmds[1]);	
	}
	else if (strncmp(cmd->cmds[0],"write",strlen("write") + 1) == 0
		&& cmd->num_cmds == 2) {
		int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
		if(! write_matrix(mats[mat1_idx]->name,mats[mat1_idx])) {
			printf("Write Failed\n");
			return;
		}
		else {
			printf("Matrix (%s) is wrote out to the filesystem\n", mats[mat1_idx]->name);
		}
	}
	else if (strncmp(cmd->cmds[0], "create", strlen("create") + 1) == 0
		&& strlen(cmd->cmds[1]) + 1 <= MATRIX_NAME_LEN && cmd->num_cmds == 4) {
		Matrix_t* new_mat = NULL;
		const unsigned int rows = atoi(cmd->cmds[2]);
		const unsigned int cols = atoi(cmd->cmds[3]);

		if(!create_matrix(&new_mat,cmd->cmds[1],rows, cols))
		{
			printf("Failed to create matrix (%s).\n", cmd->cmds[1]);
			return;
		}

		if(!add_matrix_to_array(mats,new_mat,num_mats))
		{
			printf("Failed to add matrix (%s) to the list of matrices.\n", cmd->cmds[1]);
			return;
		}

		printf("Created Matrix (%s,%u,%u)\n", new_mat->name, new_mat->rows, new_mat->cols);
	}
	else if (strncmp(cmd->cmds[0], "random", strlen("random") + 1) == 0
		&& cmd->num_cmds == 4) {
		int mat1_idx = find_matrix_given_name(mats,num_mats,cmd->cmds[1]);
		const unsigned int start_range = atoi(cmd->cmds[2]);
		const unsigned int end_range = atoi(cmd->cmds[3]);
		
		if(!random_matrix(mats[mat1_idx],start_range, end_range))
		{
			printf("Failed to create random matrix.\n");
			return;
		}

		printf("Matrix (%s) is randomized between %u %u\n", mats[mat1_idx]->name, start_range, end_range);
	}
	else {
		printf("Not a command in this application\n");
	}

}

/*
Purpose: Given a name as user input, the array of matrices is searched for a matrix with the given
	 name.
Inputs: 
	An array of preexisting matrices
	The number of matrices in the array
	The target name of the desired matrix
Return:
	If found, the position of the target matrix in the array
	If not found, -1
*/
unsigned int find_matrix_given_name (Matrix_t** mats, unsigned int num_mats, const char* target) {

	if(!mats || !(*mats))
	{
		printf("There are no matrices to search!\n");
		return -1;
	}
	if(num_mats < 0 || num_mats > 4294967295)
	{
		printf("There's something wrong with the number of matrices!\n");
		return -1;
	}
	if(!target || strcmp(target, "\n") == 0)
	{
		printf("There's no name to be searched for!\n");
		return -1;
	}

	for (int i = 0; i < num_mats; ++i) {
		if (strncmp(mats[i]->name,target,strlen(mats[i]->name)) == 0) {
			return i;
		}
	}
	return -1;
}

/*
Purpose: Any remaining matrices that exist in the array are destroyed. This is the final process
	 before program termination
Inputs: 
	An array of preexisting matrices
	The number of matrices in the list
Return: None
	
*/
void destroy_remaining_heap_allocations(Matrix_t **mats, unsigned int num_mats) {
	
	if(!mats)
	{
		printf("There's no list to destroy!\n");
		return;
	}
	if(!(*mats))
	{
		printf("There are no matrices to destroy!");
		free(mats);
		return;	
	}
		

	int i;
	for(i = 0; i < num_mats; i++)
	{
		if(!(*(mats+i)))
		{
			printf("No matrix at spot %d. Skipping...\n", i+1);
		}
		else
		{
			if(!(*mats+i)->data)
			{
				printf("There's no data in matrix %d. Freeing matrix instead...\n", i+1);
				free(*(mats+i));
			}
			else
			{
				free((*mats+i)->data);
				free(*(mats+i));
				*(mats+i) = NULL;
			}
		}	
	}
}
