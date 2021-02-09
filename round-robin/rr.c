/* rr.c
 * Hensley, Robert
 *
 * Description
 *     This is a C implementation of round robin scheduling. 
 *     Enter a quantum (in milliseconds) and a list of 
 *     commands using the format below:
 *
 *	   ./rr quantum [cmda 1 [args] [: cmdb 2 [args] [: cmdc [args] [: … ]]]]
 *	
 * Specifications
 *     - designed for unix1 machine
 *	   - MYMAXPROC defines number of commands that can be run
 *	   - MYMAXARGV defines the maximum amount of arguments
 *	   - compile with: 'gcc -Wall rr.c -o rr'
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#define MYMAXPROC 100
#define MYMAXARGV 3

#define USAGE_MESSAGE "Usage: ./p03 quantum [cmda 1 [args] [: cmdb 2 [args] [: cmdc [args] [: … ]]]]\n"

/* -- global pid -- */
pid_t child_pid;

/*

-- pid_queue functions and global variables --

* front, end, size keep track of queue indecies 

1. pq_enqueue() adds the child_pid global var to the queue
2. pq_dequeue() continues first paused pid and removes it from the queue

*/

int front = -1;
int end = -1;
int size = -1;

pid_t pid_queue[MYMAXPROC];

void pq_enqueue()
{

    // check if queue is full
    if(size > MYMAXPROC) {
        perror("exceeded MYMAXPROC limit\n");
        exit(1);
    }

    // empty queue
    if(size < 0)
    {
        pid_queue[0] = child_pid;
        front = end = 0;
        size = 1;
    }
    // wrap-around end point of queue
    else if(end == MYMAXPROC-1)
    {
        pid_queue[0] = child_pid;
        end = 0;
        size++;
    }
    else
    {
        pid_queue[end + 1] = child_pid;
        end++;
        size++;
    }
}

void pq_dequeue(struct itimerval it_val)
{

    pid_t current_pid;
    int status;

    current_pid = pid_queue[front];

    size--;
    front++;

    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }

    kill(current_pid, SIGCONT);

    waitpid(current_pid, &status, WUNTRACED);

}

/*
-- signal capture --

1. pauses the child process
2. adds child_pid to the que

*/

void pause_child(void) {

    kill(child_pid, SIGSTOP);
    pq_enqueue();

}

/*

-- run commands --

1. set timer (quantum) for process to run
2. fork and execute command
3. wait until process is complete (or stopped)

*/

void run_command(char ** cmd, struct itimerval it_val) {

    int status;

    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }

    if ( (child_pid = fork()) == 0 ) {

        execvp(cmd[0], cmd);
        exit(1);

    }

    waitpid(child_pid, &status, WUNTRACED);

}

/*

-- main code --

1. read and ocnvert quantum number
2. set SIGALARM for expired quantum
3. loop though commands and execute them
    - unfinished programs sent to queue
4. run remaining programs in queue until complete

*/

int main(int argc, char * argv[]) {
    
    int quantum;
    struct itimerval it_val;

    int argv_i;
    int cmd_i;

    char * cmd[MYMAXARGV + 1]; 

    // no arguments or only quantum argument
    if(argc<=2) {
        printf(USAGE_MESSAGE); 
        return 1;
    } 

    // convert quantum arg to itimerval value
    quantum = atoi(argv[1]);
    it_val.it_value.tv_sec = quantum / 1000;
    it_val.it_value.tv_usec = (quantum * 1000) % 1000000;
    it_val.it_interval = it_val.it_value;

    // create signal to check if quantum expired
    if (signal(SIGALRM, (void (*)(int)) pause_child) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }

    // clear memory of array
    memset(cmd, '\0', sizeof(cmd));

    // run initial commands
    cmd_i = 0;

    for(argv_i=2; argv_i < argc; argv_i++) {

        if(strcmp(argv[argv_i], ":") == 0) {

            run_command(cmd, it_val);
            memset(cmd, '\0', sizeof(cmd));
            cmd_i = 0;

        } else {
            cmd[cmd_i] = argv[argv_i];
            cmd_i++;
        }
    }

    // final command
    run_command(cmd, it_val);

    // finish paused processes until pq is empty
    while(size > 0) {
        pq_dequeue(it_val);
    }

    return 0;

}
