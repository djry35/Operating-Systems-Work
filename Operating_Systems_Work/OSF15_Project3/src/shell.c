#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include "../include/shelp.h"
#include <stdio.h>
#include <ctype.h>

#define MAX_CMD_LENGTH 1024
#define MAX_DIR_LENGTH 1024

void findRelativeDir(char*, char*);

int main(void)
{

	fprintf(stdout, "***********\nSTART SHELL\n***********\n\n\n");
	//Will hold the working directory. This will be used for pwd and ls functionalities
	char workingDir[MAX_DIR_LENGTH + 1];
	getcwd(workingDir, MAX_DIR_LENGTH + 1);	

	char relativeDir[MAX_DIR_LENGTH + 1];
	//I decided to get cute. This cuts out a portion of the absolute path directory, to be displayed to the user.
	//This is kinda like when the terminal shows you as ~/, instead of the absolute path starting at home, etc.
	//At least that's how I understand it...I could be getting cute for nothing. 
	findRelativeDir(&relativeDir[0], &workingDir[0]);

	char cmd[MAX_CMD_LENGTH + 1];
	dyn_array_t* tokens;
	while(1)
	{
		//Warning for myself more than anything else. Not really necessary, but nice to have.
		//Inception type things, ya know?
		fprintf(stdout, "\n***Warning: in dummy shell***\n");
		fprintf(stdout, "[%s]$ ", relativeDir);
		//clean out the command before we read it in. 
		memset(&cmd, 0, MAX_CMD_LENGTH + 1);
		fgets(cmd, MAX_CMD_LENGTH, stdin);
		tokens = tokenizer(cmd, " \n\0");
		
		//covers a blank command, basically. I could technically just give the prompt again, but also this
		//is nice for debugging. 
		if(!tokens)
		{
			fprintf(stdout, "Command not registered. Try again.\n");
		}
		else
		{
			char* token1;
			if(!dyn_array_extract_front(tokens, &token1))
			{
				fprintf(stdout, "Parsing error. Please restart.\n");
				//uh...free the dyn_array? idk
				return -1;
			}

			/*BEGIN BASIC IMPLEMENTATIONS*/
			if(strcmp(token1, "cd") == 0)
			{
				char* path = NULL;
				//cd expects a path, so get the path out of the tokens 
				//technically cd by itself is not an error, you just don't move out of the dir, but
				//this is my shell, so rules will be followed. 
				if(!dyn_array_extract_front(tokens, &path))
				{
					fprintf(stdout, "cd: correct use is 'cd <path>'\n");
				}
				if(chdir(path) < 0)
				{
					perror("cd");
				}	
				else
				{
					//reset the workingDir and relativeDir vars for their uses so that they have 
					//the new dir.
					memset(workingDir, 0, strlen(workingDir));
					getcwd(workingDir, MAX_DIR_LENGTH + 1);	
					findRelativeDir(&relativeDir[0], &workingDir[0]);
				}
			}
			else if(strcmp(token1, "cat") == 0)
			{
				char* file = NULL;
				//should get the file out of the tokens. 
				if(!dyn_array_extract_front(tokens, &file))
				{
					fprintf(stdout, "cat: correct use is 'cat <file path>'\n");
				}
				else
				{
					if(cat(file) < 0)
					{
						perror("cat");
					}
				}
			}
			else if(strcmp(token1, "ls") == 0)
			{
				char* path;
				//here, ls by itself is okay, it will just list the current dir. 
				if(!dyn_array_extract_front(tokens, &path))
				{
					if(ls(workingDir) < 0)
					{
						perror("ls");
					}
				}
				else
				{
					if(ls(path) < 0)
					{
						perror("ls");
					}
				}
			}
			else if(strcmp(token1, "exit") == 0)
			{
				//I wonder if there's anything to free....valgrind later.
				return 1;
			}
			else if(strcmp(token1, "pwd") == 0)
			{
				//easy. That's why workingDir is there. 
				fprintf(stdout, "%s\n", workingDir);
			}
			/*BEGIN HARD IMPLEMENTATIONS */
			else if(strcmp(token1, "join") == 0)
			{
				char* filename1 = NULL;
				char* numThreads = NULL;
				char* filename2 = NULL;
				char* column1 = NULL;
				char* column2 = NULL;
				char* outputFile = NULL;
				//if any of the above things are not in the tokens list, in the order needed, 
				//then the command wasn't used correctly. 
				//EDIT: actually, order isn't enforced here, I think. The join function will just
				//fail when the time comes.
				if(!dyn_array_extract_front(tokens, &numThreads))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				if(!dyn_array_extract_front(tokens, &filename1))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				if(!dyn_array_extract_front(tokens, &column1))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				if(!dyn_array_extract_front(tokens, &filename2))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				if(!dyn_array_extract_front(tokens, &column2))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				if(!dyn_array_extract_front(tokens, &outputFile))
				{
					fprintf(stdout, "join: correct usage is <number of threads> <filename1> ");
					fprintf(stdout, "<column1> <filename2> <column2> <output file>\n");
					continue;
				}
				//sweet, now just do the join. 
				if(join(atoi(numThreads), filename1, atoi(column1), 
					filename2, atoi(column2), outputFile) < 0)
				{
					fprintf(stdout, "Join Error.\n");
				}
				else
				{
					fprintf(stdout, "Join complete. Check output file.\n");
				}
			}
			//will get here if there is time	
			/*else if(strcmp(token1, "pfgrep") == 0)
			{
				fprintf(stdout, "pfgrep\n");
			}*/
			else
			{
				fprintf(stdout, "Command not known. Try again.\n");
			}
		}
	}	
}

//Cuts the working directory into a smaller path, for nicer viewing. 
//If the path is less than 2 directories long, it just uses that. 
void findRelativeDir(char* relativeDir, char* workingDir)
{
	char* strtok_pos = NULL;
	char* relative_pos = strtok_r(workingDir, "/", &strtok_pos);
	int i = 0; 

	//First, I will count the number of directories deep we are, just to see if I 
	//actually need to do anything. 
	while(relative_pos)
	{
		//see below about strtok. 
		*(relative_pos-1) = '/';
		relative_pos = strtok_r(NULL, "/", &strtok_pos);
		i++;
	}
	strtok_pos = NULL;
	relative_pos = strtok_r(workingDir, "/", &strtok_pos);

	//If the path is short, just use the path
	if(i <= 2)
	{
		//see below about strtok.
		if(i == 2)	*(strtok_pos - 1) = '/';

		memset(relativeDir, 0, MAX_DIR_LENGTH + 1);
		strcpy(relativeDir, relative_pos);
		return;
	}

	//Go through the string and sit the pointer just at where the last 3 directories start in the string.
	for(i; i > 3 && relative_pos; i--)
	{
		*(relative_pos-1) = '/';
		relative_pos = strtok_r(NULL, "/", &strtok_pos);
	}
	//strtok inserts '\0' into various places, so I have to repair the path string.
	*(strtok_pos-1) = '/';
	*(relative_pos - 1) = '/';
	//grab everything we need. 
	memset(relativeDir, 0, MAX_DIR_LENGTH + 1);
	strcpy(relativeDir, relative_pos);
}
