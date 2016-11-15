#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "command.h"

#define MAX_CMD_COUNT 50
#define MAX_CMD_LEN 25

/*
Purpose:
	After the user inputs a string, this function will parse out the various
	words in order to comprehend the user input. Each word will be stored in a spot in 
	an array to be later processed. 
Input:
	An input string of commands
	An array that the commands will be parsed out to
Return:
	True if commands were successfully parsed out
	False if something went wrong
*/
bool parse_user_input (const char* input, Commands_t** cmd) {
	
	if(!input || strcmp(input, "\n") == 0)
	{
		printf("There was no input!\n");
		return false;
	}
	if(!(cmd))
	{
		printf("Command array allocation error\n");
		return false;
	}
	

	char *string = strdup(input);
	
	*cmd = calloc (1,sizeof(Commands_t));
	(*cmd)->cmds = calloc(MAX_CMD_COUNT,sizeof(char*));

	unsigned int i = 0;
	char *token;
	token = strtok(string, " \n");
	for (; token != NULL && i < MAX_CMD_COUNT; ++i) {
		(*cmd)->cmds[i] = calloc(MAX_CMD_LEN,sizeof(char));
		if (!(*cmd)->cmds[i]) {
			perror("Allocation Error\n");
			return false;
		}	
		strncpy((*cmd)->cmds[i],token, strlen(token) + 1);
		(*cmd)->num_cmds++;
		token = strtok(NULL, " \n");
	}
	free(string);
	return true;
}


/*
Purpose: Once a full input string is processed, the commands that were previously processed are 
	 destroyed in order to read another string of commands. 
Input:
	An array of preprocessed commands
Return:
	Nothing
*/
void destroy_commands(Commands_t** cmd) {

	if(!(*cmd) || !cmd)
	{
		printf("There are no commands to destroy!\n");
		return;
	}	

	for (int i = 0; i < (*cmd)->num_cmds; ++i) {
		free((*cmd)->cmds[i]);
	}
	free((*cmd)->cmds);
	free((*cmd));
	*cmd = NULL;
}

