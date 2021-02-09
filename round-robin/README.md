# Round Robin Scheduling

```
rr.c
Hensley, Robert

Description
	This is a C implementation of round robin scheduling. Enter a quantum (in milliseconds) and a list of commands using the format below:
	
	./rr quantum cmd [args] : cmd [args] ... : cmd [args]
	
Specifications
	* designed for unix1 machine
	* MYMAXPROC defines number of commands that can be run
	* MYMAXARGV defines the maximum amount of arguments
	* compile with:
	
	gcc -Wall rr.c -o rr
```

