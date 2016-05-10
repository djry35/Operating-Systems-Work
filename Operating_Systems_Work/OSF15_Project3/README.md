# OSF15_Project3
Decided to say screw it. Poorly timed sickness put me too far behind. See below for notes.
Also spent a ton of time on a function that we didn't have to do, I don't think, because of
assumptions. IDK, I wanted to do it, but if I don't really get points for it, so be it. 
Oh well. I had fun while it lasted.

You will find all of my stuff in shell.c, and the start of join in shelp.c

Go to build and run the makefile. 

The makefile creates two outputs:

test: runs the test file in the test folder. Has no real function. Makes it just cause.

shell: runs the actual shell.

Commands implemented:

pwd
cd <path>
ls [file/dir]
exit
cat <file>

Commands partially implemented:
join <numThreads> <input file 1> <column 1> <input file 2> <column 2> <output file>

The command requires the given number of arguments, but doesn't actually tell the user
which arguments are incorrect before operations are done. I guess it kinda does because of
the function that checks all input parameters, but doesn't do a whole lot.

The actual shared memory part is set up properly, but no operations are actually done 
on the output file. An output file is created, but garbage is written to it. No join logic
is actually in place.

NOTE: when running ipcs, the memory shows up as not cleaned up, even though I thought
shmdt() does the job. You'll want to clean up the memory then, because apparently I didn't. 

ALSO NOTE: does not work if the files can't actually be joined. Not enough time for error
checking

There is no unit testing really for anything. No time.




