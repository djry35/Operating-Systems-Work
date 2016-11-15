Lets refresh your C memory with file reading, pointers, structs, binary, and c functions.

building the application
-----------------------------------
make 

removing the application
------------------------------------
make clean

Running the program
-------------------------------------
./matlab

Program commands
-------------------------------------

display <matrix_name>
add <first_matrix_name> <second_matrix_name_two> <matrix_result_name>
sum <matrix_name>
duplicate <src_matrix_name> <dest_matrix_name>
equal <matrix_name_one> <matrix_name_two>
shitf <matrix_name> <shift_direction> <shifts>
read <matrix_binary_file>
write <matrix_binary_file>
random <matrix_name> <start_range> <end_range>
create <matrix_name> <row_size> <col_size>

matlab usage:

The command line driven program does matrix creation, reading, writing, and other miscellaneous operations. The program automatically creates a matrix and writes that out called temp_mat (in binary do not use the cat command on it). You are able to display any matrix by using the display command. You can create a new blank matrix with the command create. To fill a matrix with random values use the random command between a range of values. To get some experience with bit shifting there is a command called shift. If you want to write and read in a matrix from the filesystem use the respective read and write commands. To see memory operations in action use the duplicate and equal commands. The others commands are sum and add. To exit the program use the exit command.


What you need to do for this assignment
--------------------------------------

You are tasked with error checking of functions, and commenting functions. Look for
TODO statements in the source files for explicit additions. We also recommend that you
get in the habit of understanding someone else's code base. If you find errors, you can fix
them and let the TAs know. This code is not unit tested, so expect possible undefined behavior.

We are hoping that this will get you back up to speed on C. If you have questions ask the TAs
as soon possible. This command line application contains important C knowledge for success in this 
class. We will have a series of quizzes over this application. Everything is fair game on
this application. So make sure to experiment with the code base with printf, understand new standard C functions,
and understand that everything is binary and datatypes are placeholders for memory sizes. 


For function comments we are looking for:

/*
 * PURPOSE:
 * INPUTS:
 * RETURN:
 **/

Example located in matrix.c starting at line 19

You also must make sure you have ZERO memory leaks. Using the tool valgrind will help.
Check the output below for what we mean by ZERO memory leaks! :D

valgrind --leak-check=yes matlab

valgrind is memory leak checking tool and does a fantastic job at finding them.

A sample output of no memory leak from your code. Commands used below included

	> create mat1 4 4
	Created Matrix (mat1,4,4)
	> create mat2 5 5
	Created Matrix (mat2,5,5)
	> exit
	==2715==
	==2715== HEAP SUMMARY:
	==2715==     in use at exit: 232,196 bytes in 538 blocks
	==2715==   total heap usage: 718 allocs, 180 frees, 259,499 bytes allocated
	==2715==
	==2715== 3 bytes in 1 blocks are definitely lost in loss record 1 of 143
	==2715==    at 0x66BB: malloc (in /usr/local/Cellar/valgrind/3.10.1/lib/valgrind/vgpreload_memcheck-amd64-darwin.so)
	==2715==    by 0x20CEAA: strdup (in /usr/lib/system/libsystem_c.dylib)
	==2715==    by 0x16448: rl_set_prompt (in /usr/lib/libedit.3.dylib)
	==2715==    by 0x168DF: readline (in /usr/lib/libedit.3.dylib)
	==2715==    by 0x100000FCB: main (in ./matlab)
	==2715==
	==2715== LEAK SUMMARY:
	==2715==    definitely lost: 3 bytes in 1 blocks
	==2715==    indirectly lost: 0 bytes in 0 blocks
	==2715==      possibly lost: 0 bytes in 0 blocks
	==2715==    still reachable: 207,082 bytes in 172 blocks
	==2715==         suppressed: 25,111 bytes in 365 blocks
	==2715== Reachable blocks (those to which a pointer was found) are not shown.
	==2715== To see them, rerun with: --leak-check=full --show-leak-kinds=all
	==2715==
	==2715== For counts of detected and suppressed errors, rerun with: -v
	==2715== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 14 from 14)

As one can see, the readline library has possible memory leaks (a fair amount on OSX). In this class one of the many goals is to create
libraries that do not have memory leaks like realine!
