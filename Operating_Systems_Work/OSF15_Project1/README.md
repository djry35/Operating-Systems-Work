# OSF15_Project1

Run the ps_tester to unit test your process scheduling library

Run the ps_analysis to do a integrated test of your process scheduling library

Run the pcb_generator to create an binary file of Process Control Blocks

To run the unit test and analysis program:

	cd build

	cmake ..

	make

RUN THE UNIT TESTER
=====

	./ps_tester <schedule_number>

Schedule numbers:

1 will test all non schedule functions and First Come First Served

2 will test all non schedule functions and Shortest Job First

3 will test all non schedule functions and Shortest Remaining Time First

4 will test all non schedule functions and Round Robin


GENERATE SYSTEM ARCHITECTURE BINARY FILES
====

	./pcb_generator <pcbFileName> <number of pcb> <random seed value>


ANALYSIS SECTION
====
	./pcb_generator TestPCBs.Bin 50 1337

Report Your Required AVG TURN AROUND TIME AND AVG LATENCY TIME inside the ANALYSIS RESULTS!

	./ps_analysis <binaryTestFile> <schedule_number> 

Scheduling numbers:

1 will read the binaryTestFile and run the First Come First Served over the read in data

2 will read the binaryTestFile and run the Shortest Job First over the read in data

3 will read the binaryTestFile and run the Shortest Remaining Time First over the read in data

4 will read the binaryTestFile and run the Round Robin over the read in data


ANALYSIS RESULTS:
====






====

