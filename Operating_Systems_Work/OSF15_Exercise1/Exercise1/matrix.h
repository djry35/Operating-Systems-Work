#ifndef _MATRIX_H_
#define _MATRIX_H_

#define MATRIX_NAME_LEN 25

typedef struct {
	char name[MATRIX_NAME_LEN];
	unsigned int rows;
	unsigned int cols;
	unsigned int *data;
}Matrix_t;

bool create_matrix (Matrix_t** new_matrix, const char* name, const unsigned int rows, const unsigned int cols);
void destroy_matrix (Matrix_t** m); 
bool write_matrix (const char* matrix_output_filename, Matrix_t* m);
bool read_matrix (const char* matrix_input_filename, Matrix_t** m);
int sum_matrix (Matrix_t* m);
bool add_matrices (Matrix_t* a, Matrix_t* b, Matrix_t* c); 
bool bitwise_shift_matrix (Matrix_t* a, char direction, unsigned int shift);
bool duplicate_matrix (Matrix_t* src, Matrix_t* dest);
bool equal_matrices (Matrix_t* a, Matrix_t* b); 
void display_matrix (Matrix_t* m); 
bool random_matrix(Matrix_t* m, unsigned int start_range, unsigned int end_range);
unsigned int add_matrix_to_array (Matrix_t** mats, Matrix_t* new_matrix, unsigned int num_mats);


#endif
