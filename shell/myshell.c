/* myshell.c
 * Hensley, Robert 
 *    
 * Description:
	
 *	`myshell` is a minimal shell that accepts
 * 	user input and executes commands with pipes
 *	
 * Specifications:
 *	- COMMAND_MAX sets the maximum characters of user input
 *	- PIPE_MAX sets the maximum number of pipes
 *	
 *	compile: `gcc -Wall -o myshell myshell.c`
 *	
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define COMMAND_MAX 1024
#define PIPE_MAX 2

// pipe index definitions
#define READ_END 0
#define WRITE_END 1

/*
    myshell_prompt

    format: 
        `{current working directory} $ `

    alternative (if getcwd(3) fails):
        `$ `

*/

void myshell_prompt(void) {
    
    char cwd[COMMAND_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s $ ", cwd);
    } 
    else {
        // default case (if getcwd fails)
        printf("$ ");
    }

}

/*
    split_by_delim

    Arguments:
        - split_str:    a string to split
        - split_list:   a target list to store tokens
        - delim_func:   a boolean function that checks for
                        a specific delimiter
    
    Logic:
        - store a list of pointers to split_str's tokens in split_list
        - fill delims with null character '\0' so tokens can be 
          null-terminated strings

    Return Value:
        - number of tokens created in the split
*/

// helper function to check for pipes
int ispipe(int c) {
    if(c == '|')
        return 1;
    return 0;
}

int split_by_delim(char * split_str, 
                    char ** split_list, 
                    int (*delim_func)(int)) {
    
    int str_i, list_i, str_len;
    
    str_len = strlen(split_str);

    for(str_i = list_i = 0; str_i < str_len; str_i++) {
            
        if(!delim_func(split_str[str_i])) {

            // add pointer of token to split_list
            split_list[list_i++] = &split_str[str_i];
            
            // find end of token by:
            //      1. the next delimeter
            //      2. the end of split_str
            while((!delim_func(split_str[str_i])) && 
                  (str_i < str_len)) 
                str_i++;

        }

        // null terminate token (or remove other delim)    
        split_str[str_i] = '\0';

    }

    // return the amount of tokens created
    return list_i;

}

/*
    myshell_cd

    - change directory in myshell
    - requires valid path argument

*/

int myshell_cd(char ** args)
{
    if(args[1] == 0) {
        fprintf(stderr, "cd: path required\n");
        return EXIT_FAILURE;
    }

    if(chdir(args[1]))
    {
        perror(args[1]);
    }

    return EXIT_SUCCESS;
}

/*
    myshell_exec

    - exec command arguments
    - redirect output appropriately using pipes

*/

void myshell_exec(char ** args,
                  int * old, int * next, 
                  int next_flag) {

    int status;

    if(fork() == 0) {

        // read from stdin
        dup2(old[READ_END], STDIN_FILENO);

        // write stdout to new pipe
        // only if another pipe is to follow
        if(next_flag > 1)
            dup2(next[WRITE_END], STDOUT_FILENO);
        
        // close old / new pipes
        close(old[0]); 
        close(old[1]);
        close(next[0]); 
        close(next[1]);
        
        // run command
        execvp(args[0], args);
        perror("");
        exit(EXIT_FAILURE);
    }

    // move output to the next pipe
    close(old[0]); 
    close(old[1]);
    old[0] = next[0]; 
    old[1] = next[1];

    wait(&status);

}

/* 
    main

    - start shell prompt
    - begin user input loop
        - create old / new pipes to pass input
        - loop through piped commands
            - run exit, cd, or command
*/

int main(void)
{

    // command strings and pipe / args pointers
    char cmd[COMMAND_MAX];
    char * pl[PIPE_MAX + 1];
    char * args[COMMAND_MAX];

    // pipe variables
    int pl_i, pl_size;
    int next_flag;
    int old[2], next[2];

    // start shell prompt
    myshell_prompt();

    // user input loop
    while(fgets(cmd, COMMAND_MAX, stdin)) {

        // create seperate strings for each 
        // command in the pipeline
        pl_size = split_by_delim(cmd, pl, ispipe);

        if (pl_size > PIPE_MAX + 1) {
            fprintf(stderr, "maximum number of pipes: %d\n", PIPE_MAX);
        }
        else {

            // open old pipe
            pipe(old);

            for(pl_i = 0; pl_i < pl_size; pl_i++) {

                next_flag = pl_size - pl_i;

                // open next pipe
                // only if 
                if (next_flag > 1)
                    pipe(next);
                
                // split arguments by whitespace
                split_by_delim(pl[pl_i], args, isspace);

                // run exit, cd, or command
                if(!strcmp(args[0], "exit")) {
                    return EXIT_SUCCESS;
                } 
                else if (!strcmp(args[0], "cd")) {
                    myshell_cd(args);
                } 
                else {
                    myshell_exec(args, old, next, next_flag); 
                }

                // clear args pointers
                memset(args, '\0', sizeof(args));
            }

        }

        // clear pl pointers
        memset(pl, '\0', sizeof(pl));

        // start shell prompt
        myshell_prompt();
    }

    // you shouldn't be here!
    return EXIT_FAILURE;
}
