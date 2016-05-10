#ifndef SHELP_H_
#define SHELP_H_

#include <dyn_array.h>
#include <stdio.h>


typedef struct result
{
	int maxLinesInOutput;
	char* buffer;
	bool isProc1Done;
	bool isProc2Done;
} result_t;

///
/// Runs ls with an optional parameter
/// \param file c-string of the file/directory/whatever you want to ls (NULL for cwd)
/// \return error code, <0 on failure, 0 on success
///
int ls(char* file);

///
/// prints contents of the secified file to stdout
/// \param file c-string of the file to print
/// \return error code, < 0 on failure, 0 on success
///
int cat(char* file);

///
/// String -> tokens -> dyn_array
/// \param str c-string to tokenize (1024 char max)
/// \parm delims c-string of delimiters
/// \return dyn_array of tokens as c-strings, NULL on error
///
dyn_array_t* tokenizer (const char* str, const char* delims);


int join(int numThreads, char* file1, int col1, char* file2, int col2, char* outputFile);
//checks to make sure all parameters for the join command are valid, and everything
//can be operated on. I don't think this amount of work was actually required for it, and
//it isn't rigorously tested, so *shrug*
bool testJoinParameters(int numThreads, char* file1, int col1, char* file2, int col2, char* outputFile
	, int* numLinesFile1, int* numLinesFile2);
void processFile(FILE* inputFile, FILE* outputFile, int searchColumn, int numThreads, int numLinesInFile, result_t* shmptr);
#endif
