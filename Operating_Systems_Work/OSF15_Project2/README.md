# OSF15_Project2

I take almost no credit, Will deserves most of it. 

There is no bonus implmementation in this submission. Did not get to it. 

Use FonefileFS_basic.h.

The bottom of the /src file contains the functions for final implementation:
	fs_remove_file()
	fs_move_file()
	restructure_directory()


To build: 
Navigate to /build
run "cmake .."
run "make"
run "FSImplementation_tester"

I believe all error checking was accomplished, but some of them are not so obvious, 
eg there is a check for moving a file into a folder that is full, but no real error messages 
were supplied (no time to put that together). 

Checks covered:
(moving a file)
	moving a file into a folder that is full
	moving a file that doesn't exist
(removing a file)
	removing a file that doesn't exist
	removing a directory that isn't empty

Checks not covered:
	moving a file to a place that already has that file
	(a crap ton more that I probably forgot)

The tests fail at removing file 2, but I'm pretty sure it's a small thing
somewhere that would make the tests go through no problem. There's some logic error 
where I'm either A) not reading the file system correctly, or B) not updating it correctly
(well, I guess those are the only two things I could mess up, oh well). File 1 doesn't even
get correctly removed, but it passes the test (it just turns into a directory, according to
Will's code). 


I have no idea what goes in a README for something I didn't build. 

I accept my fate. It was completely my fault that the project sucked, I would've been fine if 
I made even a decent attempt at taking this seriously. 

