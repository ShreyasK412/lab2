 ## UID: 005717129

## RR
This program is the implementation for round robin scheduling found in many OS systems for process handling. Given a file input containing workload and quantem length for processes, this program outputs the average waiting time and average response time for Round Robin (RR) Scheduling. 

## Building

To maintain the current build and process, follow these steps to successfully construct the program: Obtain rr.c and the Makefile from the github repository, and if desired, you can also acquire processes.txt to evaluate the program or generate your own text file with processes. Navigate to the directory where both the Makefile and rr.c are located, and execute the following commands.```
make
```

## Running
To run this program, you first require a file in the following format: 
    - first line: Count of processes run
    - following lines (processes): process id(pid), arrival time, burst time
 See the processes.txt file to see or use and example of the format,

To run the program, use the following command:
```
./rr processes.txt 4
``` 
where processes.txt can be replaced with the corresponding processes file and 4 refers to the quantum length for each process. 

## Cleaning up
To clean the program, run the following code in the same folder as the Makefile and rr.c:
```
make clean
```
