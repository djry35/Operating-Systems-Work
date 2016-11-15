#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>

#include "../include/process_scheduling.h"
#include <dyn_array.h>


/*Sooooo many things I could do with this...will save for a rainy day.*/

/*
This function does the FCFS algorithm, or in other words, a pure queue for the PCBs. 
A process is grabbed from the queue, then goes through the CPU till it's finished. This
repeats until there are no more processes. Various stats are also calculated.

Inputs: an array of future processes. They will all go through a ready Q eventually.
Returns: a structure of stats from running the algorithm. The stats will be completely empty
	 (all zeros) if an error occurred. 
*/
const ScheduleStats_t first_come_first_served(dyn_array_t* futureProcesses) {
	ScheduleStats_t stats;

	//realized that the stats var can actually have times when this 
	//fxn bombs, so I'll just have this var to return if anything goes wrong. 
	ScheduleStats_t badTimeStats = {0,0,0};

	//give me a valid queue of future procs plz.
	if(!futureProcesses)
	{
		//according to the tests, I'm supposed to return a blank stats var.
		//Seems strange, but w/e. I'd return a stats var with like -1 or something.
		return badTimeStats;
	}

	//super cheeky way to control the stats. When the first proc goes through,
	//the stats just become the time. After that, you can just add the next values
	//and divide by 2 to get an avg.
	bool isFirstProcDone = false;
		
	//will keep track of total time.
	size_t currentClockTime = 0;
	//This will be used as a timestamp for when the process goes in.
	//Subtract this from the currentClockTime after the process is done to get wallClockTime
	//Wait....this is just burst time. Oh well, will probably come in handy for other 
	//algorithms.	
	size_t startClockTime = 0;
	//the ready Q that will be filled with PCBs from the futureProcesses Q.
	dyn_array_t* rdyQ = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	
	//something went wrong with making the Q.
	if(!rdyQ)
	{
		//still don't like this return, but that's okay I guess.
		return badTimeStats;	
	}

	//makes my life easier when it comes to fetching new procs. 
	//This way, I can just read from futureProcesses until I find a proc
	//with stats higher than what I want. Everything before that gets added
	//to the Q at that time. Guessing it makes things faster, not sure.
	
	//EDIT: nevermind. Can't treat fetch_new_processes the way I want to, but
	//I can just do this one up here, since arrival time is the only thing
	//that matters.
	if(!rearranged_process_control_blocks_by_arrival_time(futureProcesses))
	{
		//didn't sort for some reason
		return badTimeStats;
	}

	//finds the processes that should start first.
	if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
	{
		//no idea what's supposed to go here.	
	}
	
	//hmmm...will there always be procs to run at time 0? I imagine so. 
	while(!dyn_array_empty(rdyQ) || !dyn_array_empty(futureProcesses))
	{
		//TODO: technically, other procs could spawn from the current proc, so do
		//those have to be sorted into the list?

		//finds the processes that should go next.
		//When this loops back up, the clock time is going to be pretty far ahead.
		//I imagine that doing this here is okay, instead of checking for new
		//procs every clock tick. Since the other procs won't run fully until
		//everything before it is done, it doesn't really matter how fast they get
		//in the Q...I think. 
		if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
		{
			//So I guess you aren't just supposed to return. Will just sit here then.
		}
		if(!dyn_array_empty(rdyQ))
		{
				//tmp guy to get the PCB that should go into the CPU next. 
			//Better way to do this? Probably
			ProcessControlBlock_t* tmp = malloc(sizeof(ProcessControlBlock_t));
			
			if(!dyn_array_extract_front(rdyQ, tmp))
			{
				return badTimeStats;
			}
			startClockTime = currentClockTime;
			//for this algorithm, the PCB goes through the CPU till it's done.
			while(tmp->burstTime > 0)
			{	
				virtual_cpu(tmp);
				//don't actually have to do anything with the process, just put it back in the queue. 	
				//so for this algorithm, it looks like the program runs until it's done. 
				//So just let it run.
				currentClockTime++;

			}	
			if(!isFirstProcDone)
			{
				//heh. I donno why I find this amusing.
				//again, no reason to have this here, but might be useful later.
				stats.averageWallClockTime = currentClockTime - startClockTime; 
				//pretty sure this works the way I want it to. 
				//actually, this is also silly, because the first proc will have 
				//latency 0.
				//EDIT: ? 
				stats.averageLatencyTime = startClockTime - tmp->arrivalTime;
				isFirstProcDone = true;
			}
			else
			{
				//average current average with the stats for the most recent proc
				stats.averageWallClockTime = ((float)currentClockTime 
					+ stats.averageWallClockTime)/2; 
				//pretty sure this works the way I want it to. 
				stats.averageLatencyTime = ((float)currentClockTime - tmp->arrivalTime
					+ stats.averageLatencyTime)/2;
			}
			free(tmp);
		}
		else
		{
			currentClockTime++;
		}
	}
	
	stats.totalClockRuntime = currentClockTime;
	return stats;
}

/*
This function does the SJF algorithm, which first sorts the ready queue by burst time ascending. 
A process is grabbed from the queue, then goes through the CPU till it's finished. This
repeats until there are no more processes. Various stats are also calculated.

Inputs: an array of future processes. They will all go through a ready Q eventually.
Returns: a structure of stats from running the algorithm. The stats will be completely empty
	 (all zeros) if an error occurred. 
*/
const ScheduleStats_t shortest_job_first(dyn_array_t* futureProcesses) {
	ScheduleStats_t stats;

	//realized that the stats var can actually have times when this 
	//fxn bombs, so I'll just have this var to return if anything goes wrong. 
	ScheduleStats_t badTimeStats = {0,0,0};

	//give me a valid queue of future procs plz.
	if(!futureProcesses)
	{
		//according to the tests, I'm supposed to return a blank stats var.
		//Seems strange, but w/e. I'd return a stats var with like -1 or something.
		return badTimeStats;
	}

	//super cheeky way to control the stats. When the first proc goes through,
	//the stats just become the time. After that, you can just add the next values
	//and divide by 2 to get an avg.
	bool isFirstProcDone = false;
		
	//will keep track of total time.
	size_t currentClockTime = 0;
	//This will be used as a timestamp for when the process goes in.
	//Subtract this from the currentClockTime after the process is done to get wallClockTime
	//Wait....this is just burst time. Oh well, will probably come in handy for other 
	//algorithms.	
	size_t startClockTime = 0;
	//the ready Q that will be filled with PCBs from the futureProcesses Q.
	dyn_array_t* rdyQ = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	
	//something went wrong with making the Q.
	if(!rdyQ)
	{
		//still don't like this return, but that's okay I guess.
		return badTimeStats;	
	}

	//finds the processes that should start first.
	if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
	{
		//log
	}
	
	//hmmm...will there always be procs to run at time 0? I imagine so. 
	while(!dyn_array_empty(rdyQ) || !dyn_array_empty(futureProcesses))
	{
		if(!dyn_array_empty(rdyQ))
		{
			if(!dyn_array_sort(rdyQ, &compareBurstTime))
			{
				//can't sort it
				//log?
			}
			//so everything below is basically the same as FCFS, just have to sort by 
			//burst time each time you need a new process.

			//tmp guy to get the PCB that should go into the CPU next. 
			//Better way to do this? Probably
			ProcessControlBlock_t* tmp = malloc(sizeof(ProcessControlBlock_t));
			
			if(!dyn_array_extract_front(rdyQ, tmp))
			{
				return badTimeStats;
			}
			startClockTime = currentClockTime;
			//for this algorithm, the PCB goes through the CPU till it's done.
			while(tmp->burstTime > 0)
			{	
				virtual_cpu(tmp);
				//don't actually have to do anything with the process, just put it back in the queue. 	
				//so for this algorithm, it looks like the program runs until it's done. 
				//So just let it run.
				currentClockTime++;
			}	
			if(!isFirstProcDone)
			{
				//heh. I donno why I find this amusing.
				//again, no reason to have this here, but might be useful later.
				stats.averageWallClockTime = currentClockTime - startClockTime; 
				//pretty sure this works the way I want it to. 
				//actually, this is also silly, because the first proc will have 
				//latency 0. 
				stats.averageLatencyTime = startClockTime - tmp->arrivalTime;
				isFirstProcDone = true;
			}
			else
			{
				//average current average with the stats for the most recent proc
				stats.averageWallClockTime = ((float)currentClockTime
					+ stats.averageWallClockTime)/2; 
				//pretty sure this works the way I want it to. 
				stats.averageLatencyTime = ((float)startClockTime - tmp->arrivalTime
					+ stats.averageLatencyTime)/2;
			}
			
			//finds the processes that should go next.
			//When this loops back up, the clock time is going to be pretty far ahead.
			//I imagine that doing this here is okay, instead of checking for new
			//procs every clock tick. Since the other procs won't run fully until
			//everything before it is done, it doesn't really matter how fast they get
			//in the Q...I think. 
			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}

			//so after the ready queue is updated, gotta find the next proc with the lowest
			//burst time, I imagine. 
			//TODO: If it's already sorted, does it return false?
			if(!dyn_array_sort(rdyQ, &compareBurstTime))
			{
				//can't sort it
				//log
			}
		}
		else
		{
			currentClockTime++;
			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}
		}
	}
	
	stats.totalClockRuntime = currentClockTime;
	return stats;

}

const ScheduleStats_t shortest_remaining_time_first(dyn_array_t* futureProcesses) {

	//no preemption, so copy paste away!
	//This might actually be using a quantum, need to look up.
	//----EDIT: this has to be using a quantum, otherwise it's exactly like SJF....
	//----EDIT 2: nevermind! New processes are added to queue while proc is running
	//through CPU, so check to see if any new process that came is is shorter than 
	//current.	

	ScheduleStats_t stats;

	//realized that the stats var can actually have times when this 
	//fxn bombs, so I'll just have this var to return if anything goes wrong. 
	ScheduleStats_t badTimeStats = {0,0,0};

	//give me a valid queue of future procs plz.
	if(!futureProcesses)
	{
		//according to the tests, I'm supposed to return a blank stats var.
		//Seems strange, but w/e. I'd return a stats var with like -1 or something.
		return badTimeStats;
	}

	//super cheeky way to control the stats. When the first proc goes through,
	//the stats just become the time. After that, you can just add the next values
	//and divide by 2 to get an avg.
	bool isFirstProcDone = false;
		
	//will keep track of total time.
	size_t currentClockTime = 0;
	//This will be used as a timestamp for when the process goes in.
	//Subtract this from the currentClockTime after the process is done to get wallClockTime
	//Wait....this is just burst time. Oh well, will probably come in handy for other 
	//algorithms.	
	size_t startClockTime = 0;
	//the ready Q that will be filled with PCBs from the futureProcesses Q.
	dyn_array_t* rdyQ = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	
	//something went wrong with making the Q.
	if(!rdyQ)
	{
		//still don't like this return, but that's okay I guess.
		return badTimeStats;	
	}

	//finds the processes that should start first.
	if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
	{
		//log
	}


	//do an initial sort to get the quickest process
	if(!dyn_array_sort(rdyQ, &compareBurstTime))
	{
		//log?
		//return badTimeStats;
	}
	
	//hmmm...will there always be procs to run at time 0? I imagine so. 
	while(!dyn_array_empty(rdyQ) || !dyn_array_empty(futureProcesses))
	{
		if(!dyn_array_empty(rdyQ))
		{
			//so everything below is basically the same as FCFS, just have to sort by 
			//burst time each time you need a new process.

			//tmp guy to get the PCB that should go into the CPU next. 
			//Better way to do this? Probably
			ProcessControlBlock_t* tmp = malloc(sizeof(ProcessControlBlock_t));
			
			if(!dyn_array_extract_front(rdyQ, tmp))
			{
				return badTimeStats;
			}
			startClockTime = currentClockTime;
			//for this algorithm, the PCB goes through the CPU till it's done.
			while(tmp->burstTime > 0)
			{	
				virtual_cpu(tmp);
				//don't actually have to do anything with the process, just put it back in the queue. 	
				//so for this algorithm, it looks like the program runs until it's done. 
				//So just let it run.
				currentClockTime++;
			
				if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
				{
					//something went wrong with fetch_new_procs
					//return badTimeStats;
					//log?
				}
				
				if(!dyn_array_sort(rdyQ, &compareBurstTime))
				{
					//log?
					//return badTimeStats;
				}

				if(((ProcessControlBlock_t*)dyn_array_at(rdyQ, 0))->burstTime < tmp->burstTime)
				{
					if(!dyn_array_push_back(rdyQ, tmp))
					{
						return badTimeStats;
					}
					
					if(dyn_array_extract_front(rdyQ, tmp))
					{
						return badTimeStats;
					}
				}

			}	
			if(!isFirstProcDone)
			{
				//heh. I donno why I find this amusing.
				//again, no reason to have this here, but might be useful later.
				stats.averageWallClockTime = currentClockTime - startClockTime; 
				//pretty sure this works the way I want it to. 
				//actually, this is also silly, because the first proc will have 
				//latency 0. 
				stats.averageLatencyTime = startClockTime - tmp->arrivalTime;
				isFirstProcDone = true;
			}
			else
			{
				//average current average with the stats for the most recent proc
				stats.averageWallClockTime = ((float)currentClockTime
					+ stats.averageWallClockTime)/2; 
				//pretty sure this works the way I want it to. 
				stats.averageLatencyTime = ((float)startClockTime - tmp->arrivalTime
					+ stats.averageLatencyTime)/2;
			}
			
			//finds the processes that should go next.
			//When this loops back up, the clock time is going to be pretty far ahead.
			//I imagine that doing this here is okay, instead of checking for new
			//procs every clock tick. Since the other procs won't run fully until
			//everything before it is done, it doesn't really matter how fast they get
			//in the Q...I think. 
			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}


			//so after the ready queue is updated, gotta find the next proc with the lowest
			//burst time, I imagine. 
			if(!dyn_array_sort(rdyQ, &compareBurstTime))
			{
				//can't sort it
				//return badTimeStats;
				//log?
			}
		}
		else
		{
			currentClockTime++;
			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}
		}
	}
	
	stats.totalClockRuntime = currentClockTime;
	return stats;


}

/*
This function does the round-robin algorithm, which has the processes run for a specific length of time before
the next process has a turn.
I think normally the processes are added to the queue by priority, but we don't have that, so I'm just going
to do arrival time. 
A process is grabbed from the queue, then goes through the CPU till it's finished. This
repeats until there are no more processes. Various stats are also calculated.

Inputs: an array of future processes. They will all go through a ready Q eventually.
Returns: a structure of stats from running the algorithm. The stats will be completely empty
	 (all zeros) if an error occurred. 
*/
const ScheduleStats_t round_robin(dyn_array_t* futureProcesses, const size_t quantum) {

	//a good chunk can be copy pasted.
	
	ScheduleStats_t stats;

	//realized that the stats var can actually have times when this 
	//fxn bombs, so I'll just have this var to return if anything goes wrong. 
	ScheduleStats_t badTimeStats = {0,0,0};

	//give me a valid queue of future procs plz.
	if(!futureProcesses)
	{
		//according to the tests, I'm supposed to return a blank stats var.
		//Seems strange, but w/e. I'd return a stats var with like -1 or something.
		return badTimeStats;
	}

	//super cheeky way to control the stats. When the first proc goes through,
	//the stats just become the time. After that, you can just add the next values
	//and divide by 2 to get an avg.
	bool isFirstProcDone = false;

	//will keep track of total time.
	size_t currentClockTime = 0;
	
	//the ready Q that will be filled with PCBs from the futureProcesses Q.
	dyn_array_t* rdyQ = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	
	//something went wrong with making the Q.
	if(!rdyQ)
	{
		//still don't like this return, but that's okay I guess.
		return badTimeStats;	
	}

	//initial sort is by arrival time, I would guess. This is likely up to the programmer.	
	//Although, you would want to do this by priority, but there is no priority here.
	if(!rearranged_process_control_blocks_by_arrival_time(futureProcesses))
	{
		return badTimeStats;
	}

	//finds the processes that should start first.
	if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
	{
		//log
	}
	
	//for the first process, it will always be 0, so I can initialize this okay.
	stats.averageLatencyTime = 0;	

	//will hold the guy that's coming off the array and into the CPU.
	ProcessControlBlock_t* dummy = malloc(sizeof(ProcessControlBlock_t));

	//hmmm...will there always be procs to run at time 0? I imagine so. 
	while(!dyn_array_empty(rdyQ) || !dyn_array_empty(futureProcesses))
	{
		if(!dyn_array_empty(rdyQ))
		{
			if(!dyn_array_extract_front(rdyQ, dummy))
			{
				return badTimeStats;
			}
		
			//basically, has the currentClockTime incremented to the next multiple of
			//quantum. But, the process has to run that last tick in the quantum range,
			//so increment after the check is done. 
			//BUT: does the next process not run the full quantum, because the clock for the
			//next process starts at 1, not 0? Will wait for test results.
			while(currentClockTime++ % quantum != 0)
			{
				virtual_cpu(dummy);
			}
		
			//not actually sure if this is needed, will test.
			if(dummy->burstTime == 0)
			{
				//If this is the first process to finish, its time to finish is the baseline.
				if(!isFirstProcDone)
				{
					stats.averageWallClockTime = currentClockTime;
					isFirstProcDone = true;
				}
				else
				{
					stats.averageWallClockTime = (stats.averageWallClockTime + currentClockTime) / 2;
				}
			}
			//I would guess that the current process is pushed back into the queue
			//before the futureProcs go into the queue, since the current process
			//will have an earlier arrival time.
			else if(!dyn_array_push_back(rdyQ, dummy))
			{
				return badTimeStats;
			}	

			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}
		
			//this is right before the next processes is brought out of the rdy Q, so here should be fine.
			stats.averageLatencyTime = (stats.averageLatencyTime + currentClockTime) / 2;
		}
		else
		{
			currentClockTime++;
			if(!fetch_new_processes(rdyQ, futureProcesses, currentClockTime))
			{
				//log
			}
		}
	}
	
	stats.totalClockRuntime = currentClockTime;
	return stats;



} 


/*
Easily the most confusing fxn of this assignment. Still not entirely sure what this does.
EDIT: okay, so this function will fork this process (of schedules) X number of times,
where X is the number of elements in the futureProcesses dyn_array. Why? Who knows.

Input:
	An array of PCBs
Return:
	true on success
	false for any failure
*/
const bool create_suspended_processes_and_assign_pcbs(dyn_array_t* futureProcesses){

	if(!futureProcesses)
	{
		return false;
	}

	size_t sizeOfArray = dyn_array_size(futureProcesses);
	if(sizeOfArray == 0)
	{
		//technically also true, cause I added 0 processes to futureProcesses,
		//which is also the size of the dyn_array...
		return false;
	}

		
	//sketchy use of int...
	for(int i = 0; i < sizeOfArray; i++)
	{

		pid_t tmp;
		
		//So I assume that tmp will be shared across each child	
		tmp = fork();
		//newly discovered check on if fork was successful?
		if(tmp == -1)
		{
			return false;
		}
		//this is if I'm the child.
		if(tmp == 0)
		{
			//the copied heap needs to be freed up, otherwise that's 
			//wasted space I think. 
			//EDIT: COWs
			dyn_array_destroy(futureProcesses);
			//here's your loop?
			while(1) {}
		}
		//otherwise I'm the parent.
		else
		{
			//I suspend the proc?
			if(kill(tmp, SIGSTOP) < 0)
			{
				//uh, well I can't stop it, so I think I'm screwed.
				return false;
			}
			//can probably be dyn_array_insert_back, or something like that
			if(!dyn_array_at(futureProcesses, i))
			{
				//log?
				return false;
			}
			else
			{
				//WAIT, THIS IS WHAT YOU ARE SUPPOSED TO DO?!
				((ProcessControlBlock_t*)dyn_array_at(futureProcesses, i))->pid = tmp;
			}
			//check to see if I need to remalloc, or if dyn_array_insert
			//just does a memcpy. 
			//Edit: I'll find this out in the test I guess.
		}
		//is this it? 
	}
	
	return true; 
}

/*
From the array of future processes, there are going to eventually be processes that are ready to 
enter the ready Q. For whatever reason, processes are scheduled to run in the future, so they have
to be readied when the time comes. The futureProcesses that come in are sorted based on the
characteristic of the scheduling algorithm, so by going down the list of futureProcesses, PCBs
are readied in the correct order (with correct priority, etc).

Inputs: an array representing the ready Q which will be added to, an array of future processes to be
checked for ready processes, and the current clock time to use in the comparison.
Returns: false for any error, true on full success.
*/
const bool fetch_new_processes(dyn_array_t* newProcesses, dyn_array_t* futureProcesses, const size_t currentClockTime) {
	
	if(!futureProcesses || !newProcesses || currentClockTime < 0)
	{
		//absolutely no clue how I can get clock time < 0, but just in case.
		return false;
	}
	
	size_t i = 0;
	size_t size = dyn_array_size(futureProcesses);
	if(size == 0)
	{
		//uh...what?
		return false;
	}
	
	ProcessControlBlock_t childProc;

	//Is there seriously no better way to do this? I guess not, but I imagine that 
	//you'd want different functions for this based on the scheduling algorithm,
	//so that you can stop the for loop when you know you can. 
	for(; i < size; i++)
	{
		//I guess you can have processes in the futureProcs list that have arrival time < currentClockTime.
		//Not sure why though...wouldn't that always be kept up to date?
		if(((ProcessControlBlock_t*)dyn_array_at(futureProcesses, i))->arrivalTime <= currentClockTime)
		{
			if(!dyn_array_extract(futureProcesses, i, &childProc))
			{
				return false;
			}
			
			//the size has to be updated so that i doesn't go out of bounds at the end of
			//the dyn_array.
			
			size = dyn_array_size(futureProcesses);
			i--;

			if(!dyn_array_push_back(newProcesses, &childProc))
			{
				return false;
			}
			//still have no idea if the dyn_array functions do a memcpy. We'll see I guess.		
		}
	}

	//TODO: note to self
	//not a clue why this would be here. This wasn't specified in the document, but no other way to
	//have it pass test 29.
	if(dyn_array_size(newProcesses) == 0)
	{
		return false;
	}

	//this was false...why
	return true;
}

/*
A core that does nothing, relatively speaking. Was expecting something super fancy, was 
sorely disappointed. This thing literally just starts and stop processes, and that is what 
takes up the burst time. The core will update the burst time of the processes automatically.

Inputs: a process that is ready for the core.
Returns: nothing.
*/
void virtual_cpu(ProcessControlBlock_t* runningProcess) {
	
	if (runningProcess == NULL || !runningProcess->pid) {
		return;	
	}
	ssize_t err = 0;
	err = kill(runningProcess->pid,SIGCONT);
	if (err < 0) {
		fprintf(stderr,"VCPU FAILED to CONT process %d\n",runningProcess->pid);
	}
	--runningProcess->burstTime;
	err = kill(runningProcess->pid,SIGSTOP);

	if (err < 0) {
		fprintf(stderr,"VCPU FAILED to STOP process %d\n",runningProcess->pid);
	}
	
}

/*
Reads incoming data from an input file, and adds the PCBs to future processes. 

Inputs:
	an array of PCBs that will get more processes as they are read from file.
	a path to the binary input file. The file contents are as follows:
		<number of PCBs in file>
		<PCB_1 data structure contents>
		<PCB_2 data structure contents>
		... (for number of PCBs)
Return:
	false on any failure
	true for complete success

*/
const bool load_process_control_blocks_from_file(dyn_array_t* futureProcesses, const char* binaryFileName)
{
	uint32_t numProcs;

	//should probably send me stuff I can use
	if(!binaryFileName || !futureProcesses)
	{
		return false;
	}

	//opens the file. 
	int fd = open(binaryFileName, O_RDONLY, S_IRUSR | S_IRGRP);
	//if the file was opened succesfully, fd will be > 0 most likely, although not sure when it's 0. But definitely >=0. 
	if(fd >= 0)
	{
		//checking to see if a size can be read from the file, as well as if the size in the file is greater than the size of the array
		//TODO: see if there's a fxn that increases the size of the array?
		//TODO: for some reason, I can't make sure that numProcs < dyn_array_size(futureProcs)...check on this.
		if(read(fd, &numProcs, sizeof(uint32_t)) == sizeof(uint32_t)) 
		{
			//Buffer for the file reading
			ProcessControlBlock_t* tmp = malloc(sizeof(ProcessControlBlock_t));
			if(!tmp)
			{
				//not sure if I should keep trying to get space...just a return for now.
				//TODO: see if there are other options (timeout?).
				return false;
			}
			//don't know my types well enough...TODO: should i be a uint32_t?
			for(int i = 0; i < numProcs; i++)
			{
				//Can I read a full PCB from the file?
				if(read(fd, (void*)tmp, sizeof(ProcessControlBlock_t)) == sizeof(ProcessControlBlock_t))
				{
					//Not sure if this is the correct way to do this, but we'll find out
					if(!dyn_array_insert(futureProcesses, i, tmp))
					{	
						//No idea how to handle this error. I guess just return.
						//TODO: research
						return false;
					}
				}
				else
				{
					//Nope.
					//Note: I assume that we should add the PCBs we can read from the file, and just not any others. 
					//TODO: Check on this...maybe just keep reading the file? Idk.
					return false;
				}
			}
			//gotta close the file
			close(fd);
			//if(close(fd) == 0)
			{
				return true;
			}
			//Not sure if this is good enough...ideally you'd want to know the errno or something to be able to fix it later. 
			return false;
		}
		//Nope.
		return false;
	}
	//Nope.
	return false;

}

const bool rearranged_process_control_blocks_by_arrival_time(dyn_array_t* futureProcesses) {
	
	//...lol? this it?
	if(!futureProcesses)
	{
		return false;
	}	

	if(!dyn_array_sort(futureProcesses, &compareArrivalTime))
	{
		return false;
	}
	
	return true;
} 


int compareBurstTime(const void* a, const void* b)
{
	const size_t aBT = ((ProcessControlBlock_t*)a)->burstTime;
	const size_t bBT = ((ProcessControlBlock_t*)b)->burstTime;
	return bBT - aBT;
}

int compareArrivalTime(const void* a, const void* b)
{
	const size_t aAT = ((ProcessControlBlock_t*)a)->arrivalTime;
	const size_t bAT = ((ProcessControlBlock_t*)b)->arrivalTime;
	return bAT - aAT;
}
