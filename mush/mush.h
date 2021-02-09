#ifndef MUSHH
#define MUSHH

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/* Command Line Limits */
#define PIPELINE_MAX 512
#define PIPE_MAX 10
#define ARGS_MAX 10

/* Pipe Stuff */
#define WRITE_END 1
#define READ_END 0

/* Various Strings */
#define LINE "--------\n"
#define USAGE "usage: ./demo [ scriptfile ]\n"

/* Data Structures */

typedef struct stage_st stage;

struct stage_st {
    int argc;
    char * name;
    char * argv[ARGS_MAX];
};

typedef struct pipeline_st pipeline;

struct pipeline_st {
    int stgc;
    stage * stgv[PIPE_MAX];
    char * input;
    char * output;
};

#endif