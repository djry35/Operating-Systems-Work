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
#define MAX_BUFFER_RD_SIZE 1024
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

// private function prototypes
int dirwalk(char *dir);

int ls(char *file) {
    if (!file) {
        char *cwd;
        char buff[PATH_MAX + 1];
        cwd = getcwd(buff, PATH_MAX + 1);
        if (dirwalk(cwd) < 0) {
            return -1;
        }
    } else {

        struct stat fileStat;
        if (stat(file, &fileStat) < 0)
            return -1;
        if (!S_ISDIR(fileStat.st_mode)) {
            fprintf(stdout, "%s\n", file);
            return 0;
        } else {
            if (dirwalk(file) < 0) {
                return -1;
            }
        }
    }
    return 0;
}

int cat(char *file) {
    if (!file) {
        return -1;
    }
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    char buffer[MAX_BUFFER_RD_SIZE];
    size_t bufferSize = 0;
    bufferSize = read(fd, buffer, MAX_BUFFER_RD_SIZE);
    while (bufferSize >= 1) {
        write(2, buffer, bufferSize);
        bufferSize = read(fd, buffer, MAX_BUFFER_RD_SIZE);
    }
    close(fd);
    return 0;
}

void dyn_tok_destruct(void *tok_str) {
    if (tok_str) {
        free(*(char **)tok_str);
    }
}

dyn_array_t *tokenizer(const char *str, const char *delims) {
    // Haha, oh man, this is gross
    dyn_array_t *strings = NULL;
    char *strtok_pos = NULL; // strtok_r uses a pointer of ours to keep track of data, allowing us to be thread safe
    if (str && delims) {
        char *str_copy = strndup(str, MAX_BUFFER_RD_SIZE); // buffer created will be 1024+terminator
        if (str_copy) {
            strings = dyn_array_create(16, sizeof(char *), &dyn_tok_destruct);
            if (strings) {
                char *tok_pos = strtok_r(str_copy, delims, &strtok_pos);
                while (tok_pos != NULL) {
                    char *temp_ptr = strndup(tok_pos, MAX_BUFFER_RD_SIZE);
                    if (temp_ptr) {
		        if (dyn_array_push_back(strings, &temp_ptr)) {
                       	   tok_pos = strtok_r(NULL, delims, &strtok_pos);
	                   continue;           
			}
                        free(temp_ptr);
                        dyn_array_destroy(strings);
                        strings = NULL;
                    }
                    break;
                }
            }
            free(str_copy);
        }
    }
    return strings;
}

/*
    PRIVATE FUNCTIONS
*/
int dirwalk(char *dir) {
    if (!dir) {
        return -1;
    }

    struct dirent *dp;
    DIR *dfd;

    if ((dfd = opendir(dir)) == NULL) {
        fprintf(stdout, "ls: can't open %s\n", dir);
        return -1;
    }
    while ((dp = readdir(dfd)) != NULL) {
        fprintf(stdout, "%s\n", dp->d_name);
    }
    closedir(dfd);
    return 0;
}

int join(int numThreads, char* file1, int col1, char* file2, int col2, char* outputFile)
{
	int numLinesFile1, numLinesFile2;
	int numThreadsFile1, numThreadsFile2;

	bool isChild1Done = false, isChild2Done = false;

	if(!testJoinParameters(numThreads, file1, col1, file2, col2, outputFile, &numLinesFile1, &numLinesFile2))
	{
		return -1;
	}

	key_t key = 123456; //value doesn't really matter I think.
	result_t r; 
	r.isProc1Done = false;
	r.isProc2Done = false;
	r.buffer = NULL;
	//went ahead and allocated the maximum output possibility. The buffer could be filled if both files are 
	//identical, I think? Or if you have one column identical, and the rest of the data is different, but 
	//spans across the maximum line length
	r.maxLinesInOutput = numLinesFile1 + numLinesFile2;
	//1024 is max line length, but if both lines are concatenated, need 1024*2
	r.buffer = malloc(sizeof(char)*r.maxLinesInOutput*1024*2);
		
	int id = shmget(key, sizeof(r), IPC_CREAT | 0666); 
	if(id < 0) 
	{
		fprintf(stdout, "Failure to create shared memory\n");
		return -1;
	}

	//TODO: error check	
	result_t* shmptr = shmat(id, NULL, 0);
	//I think this is what I want to do...
	memcpy(shmptr, &r, sizeof(result_t));
	
	//okay, so now I have a pointer to the shared memory, and the values are what I want. Then each 
	//proc will get a copy of this address, and can write to it using a semaphore...right?


	FILE* fp1 = fopen(file1, "r");
	FILE* fp2 = fopen(file2, "r");
	FILE* fp3 = fopen(outputFile, "w");
	
	int pids[2] = {0};
	
	int i;
	//So the parent is gonna run this loop, and each of the children will break out of the loop and do their
	//thing. When the parent is done, it will wait until the children are done. 
	for(i = 0; i < 2; i++)
	{
		pids[i] = fork();
		//The child gets the value 0. The parent gets the actual pid of the child. 
		//The pids will be held onto for possible later use. 
		if(pids[i] == 0)
		{
			//because of the logic below, we need a nonzero value that is known. 
			//fork() returns -1 on error, so I'll use -2.
			//couldn't think of a better way to hold onto the pids while at the same time
			//knowing that the fork was a success.  
			pids[i] = -2;
			break;
		}
		else if(pids[i] == -1)
		{
			return -1;
		}
	}

	//parent
	if(pids[0] != -2 && pids[1] != -2)
	{
		//wait for the children to be done
		while(!shmptr->isProc1Done || !shmptr->isProc2Done);

		fprintf(fp3, "%s", shmptr->buffer);
	
		fclose(fp3);
		
		free(shmptr->buffer);	

		shmdt(shmptr);
		
		//maybe should use SIGKILL, idk
		if(kill(pids[0], SIGSTOP) < 0)
		{
			perror("child1 kill");
		}
		if(kill(pids[1], SIGSTOP) < 0)
		{
			perror("child2 kill");
		}
		return 0;
	}
	//child 2
	else if(pids[0] != -2 && pids[1] == -2)
	{
		processFile(fp2, fp3, col2, numThreadsFile2, numLinesFile2, shmptr, 2);
		fclose(fp2);
		//guess I can let the parent close the children on its own.
		shmptr->isProc2Done = true;
		//my way of handling signals. The structure has a boolean the parent will 
		//continually check until the boolean for each process is true.
		while(1);
	}
	//child 1
	else if(pids[0] == -2 && pids[1] != -2)
	{
		processFile(fp1, fp3, col1, numThreadsFile1, numLinesFile1, shmptr, 1);
		fclose(fp1);
		//signal
		//see child 2.
		shmptr->isProc1Done = true;
		while(1);
	}
	
	//shouldn't get here?
	return -3;
}

//Makes sure that everything is okay. 
bool testJoinParameters(int numThreads, char* file1, int col1, char* file2, int col2, char* outputFile, int* numLinesFile1, int* numLinesFile2)
{
	//First param check.
	if(!file1 || !file2 || !outputFile)
	{
		fprintf(stdout, "Join: invalid filename\n");
		return false;
	}	

	if(numThreads < 1)
	{
		fprintf(stdout, "Join: number of threads needs to be more than 0.\n");
		return false;
	}

	if(numThreads > 25)
	{
		fprintf(stdout, "Warning: number of threads is a little high, don't you think?\n");
	}

	if(col1 < 0 || col2 < 0)
	{
		fprintf(stdout, "Join: invalid column value\n");
	}
	
	//check to see if we can open the first file. 
	FILE* fp1 = fopen(file1, "r");
	if(!fp1)
	{
		perror("(Join) file1 reading error");
		return false;
	}

	//alright, the messy part. Go through the csv file and make sure it's okay. 
	//Unclear if this should be implied that this is okay, but doing it anyway. 
	char c;
	int colCount = 0; //count the columns in the line that is being parsed. Each value surrounded by ',' is a column.
	int lineCount = 0; //count the number of lines in the file
	int lineLength = 0; //count the length of the current line being parsed
	bool atStart = true; //makes sure that the start of the file is okay. Just for completion's sake. 
	bool containsColumn = false;
	while((c = fgetc(fp1)) != EOF)
	{
		//Now, go through each character in the file and figure out what we're doing.

		//Found a column. I will assume that the end of the line doesn't have stray characters. 
		if(c == ',')
		{
			if(atStart)
			{
				fprintf(stdout, "Join: file1 improperly formatted.\n");
				fclose(fp1);
				return false;
			}
			if(lineLength == 1024)
			{
				fprintf(stdout, "Join: file1 has lines that are too long.\n");
				fclose(fp1);
				return false;
			}
			else
			{
				lineLength++;
				colCount++;
				continue;
			}
		}
		//Found the end of a line. 
		else if(c == '\n')
		{
			if(atStart)
			{
				fprintf(stdout, "Join: file1 improperly formatted.\n");
				fclose(fp1);
				return false;
			}
			else if(lineCount == 5000000)
			{
				fprintf(stdout, "Join: file1 has too many lines.\n");
				fclose(fp1);
				return false;
			}
			//> or >= ? Is the user smart enough to start at col 0? If I assume user will enter 
			//column 1 instead of column 0, should be >. 
			if(colCount >= col1)
			{
				containsColumn = true;
				continue;
			}
			else
			{
				colCount = 0;
				lineCount++;
				lineLength = 0;
				continue;
			}
		}
		//Found some sort of character in the file that would be part of the column value. 
		//Assuming it should be just alphanumeric. Don't know if random other characters are allowed. 
		else if(isalnum(c) != 0)
		{
			//A line is too long
			if(lineLength == 1024)
			{
				fprintf(stdout, "Join: file1 has lines that are too long.\n");
				fclose(fp1);
				return false;
			}
			//This should be the first thing that we find in the file, so only here will
			//we know at least the start of the file is properly formatted.
			atStart = false;
			lineLength++;
			continue;
		}
		//Found something weird. Idk. 
		else
		{
			fprintf(stdout, "Join: file1 parsing error. Check file.\n");
			fclose(fp1);
			return false;
		}
	}

	if(!containsColumn)
	{
		fprintf(stdout, "Join: file1 does not contain column %d.\n", col1);
		fclose(fp1);
		return false;
	}
	
	fclose(fp1);

	//Max number of lines a thread can process is 50, so check that.	
	if(lineCount / numThreads > 50)
	{
		fprintf(stdout, "Join: too many columns in file1 for %d threads to process.\n", numThreads);
		return false;
	}
		
	//will be used back in join.
	*numLinesFile1 = lineCount;

	//Reset everything and check file 2 for the correct stuff just like above. 
	atStart = true;
	colCount = 0;
	lineCount = 0;
	lineLength = 0;	
	containsColumn = false;

	FILE* fp2 = fopen(file2, "r");
	if(!fp2)
	{
		perror("(Join) file1 reading error");
		return false;
	}

	while((c = fgetc(fp2)) != EOF)
	{
		if(c == ',')
		{
			if(atStart)
			{
				fprintf(stdout, "Join: file2 improperly formatted.\n");
				fclose(fp2);
				return false;
			}
			if(lineLength == 1024)
			{
				fprintf(stdout, "Join: file2 has lines that are too long.\n");
				fclose(fp2);
				return false;
			}
			else
			{
				lineLength++;
				colCount++;
				continue;
			}
		}
		else if(c == '\n')
		{
			if(atStart)
			{
				fprintf(stdout, "Join: file2 improperly formatted.\n");
				fclose(fp2);
				return false;
			}
			else if(lineCount == 5000000)
			{
				fprintf(stdout, "Join: file2 has too many lines.\n");
				fclose(fp2);
				return false;
			}
			//> or >= ? Is the user smart enough to start at col 0?
			if(colCount >= col2)
			{
				containsColumn = true;
			}
			else
			{
				colCount = 0;
				lineCount++;
				lineLength = 0;
				continue;
			}
		}
		else if(isalnum(c) != 0)
		{
			if(lineLength == 1024)
			{
				fprintf(stdout, "Join: file2 has lines that are too long.\n");
				fclose(fp2);
				return false;
			}
			atStart = false;
			lineLength++;
			continue;
		}
		else
		{
			fprintf(stdout, "Join: file2 parsing error. Check file.\n");
			fclose(fp2);
			return false;
		}
	}

	if(!containsColumn)
	{
		fprintf(stdout, "Join: file2 does not contain column %d.\n", col2);
		fclose(fp2);
		return false;
	}
	
	fclose(fp2);
	
	if(lineCount / numThreads > 50)
	{
		fprintf(stdout, "Join: too many columns in file2 for %d threads to process.\n", numThreads);
		return false;
	}
	
	*numLinesFile2 = lineCount;

	//Yes! Everything checks out!
	return true;	
}

void processFile(FILE* inputFile, FILE* outputFile, int searchColumn, int numThreads, int numLinesInFile, result_t* shmptr)
{
/*	
start of chicken scratch

	char buffer[1024];
	while(!feof(inputFile))
	{
		fscanf(inputFile, "%s", buffer);
		char* token_pos = NULL;
		token_pos = strtok_r(buffer, ",\n", &token_pos);
		if(!token_pos)
		{
			return;
		}
		else
		{
			//compare to shared mem
			token_pos = strtok_r(NULL, ",\n", &token_pos);
		}
	}
*/		
	return;
}
