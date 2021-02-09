#ifndef MYTAR
#define MYTAR

#define _BSD_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* Unix I/O Libraries */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Header Info Libraries */
#include <dirent.h>
#include <sys/time.h>
#include <utime.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define PATHMAX 256
#define BLOCK_SIZE 512
#define PAD_SIZE 1024

#define HEADER_COUNT 16

#define HEADER_LENGTHS  100, 8, 8, 8, 12, 12, 8, 1, \
                        100, 6, 2, 32, 32, 8, 8, 155

#define HEADER_OFFSETS  0, 100, 108, 116, 124, 136, \
                        148, 156, 157, 257, 263,    \
                        265, 297, 329, 337, 345

/* Messages */
#define NO_ARGS_MESSAGE "you must specify at least \
one of the 'ctx' options.\n"

#define TOO_MANY_OPTS_MESSAGE "you may only choose one of \
the 'ctx' options.\n"

#define USAGE_MESSAGE "usage: //home/pn-cs357/demos/mytar \
[ctxSp[f tarfile]] [file1 [ file2 [...] ] ]\n"

#define F_MESSAGE "./tests/mytar: option 'f' requires an \
archive name.\n"

#define BOGUS "Malformed header found.  Bailing.\n"

/* remove this later */
typedef int bool;
enum {FALSE, TRUE}; /* Boolean Vals */
                        
void * safe_malloc(int size) 
{ 
    void *new;
    new = malloc(size);
    if (new == NULL) 
    {
        perror("malloc failure");
        exit(-1); 
    }
    return new; 
}

void * safe_realloc(void * obj, int size) 
{ 
    void * new;
    new  = realloc(obj, size);
    if (new == NULL) 
    {
        perror("realloc failure");
        exit(-1); 
    }
    return new;
}

void read_header(int infd, int size, uint8_t * content)
{
    int i = 0;
    uint8_t * buff;

    buff = safe_malloc(sizeof(uint8_t));

    for(; i < size; i++)
    {
        if((read(infd, buff, sizeof(uint8_t))) < 0)
        {
            perror("read error");
            exit(-1);
        }
        content[i] = *buff;
    }
    free(buff);
}

void path_concat(uint8_t * name, uint8_t * prefix, uint8_t * path)
{
    /* checks if name is 100 in case it's not NULL terminated */
    if((strlen(prefix) == 0))
    {
        strncpy(path, name, 100);
        path[100] = '\0';
    }
    /* must use strncpy to return 100 chars of name 
    (overflow name is not NULL terminated) */
    else
    {
        strcpy(path, prefix);
        path[strlen(prefix)] = '/';
        path[strlen(prefix) + 1] = '\0';
        strcat(path, name);
    }
}

/* header struct: holds the contents of the header in uint8_t arrays */
typedef struct header_st header;

struct header_st 
{
    uint8_t name[100];      /* NUL-terminated if NUL fits       */
    uint8_t mode[8];        
    uint8_t uid[8];         
    uint8_t gid[8];         
    uint8_t size[12];       
    uint8_t mtime[12];      
    uint8_t chksum[8];      
    uint8_t typeflag[1];    
    uint8_t linkname[100];  /* NUL-terminated if NUL fits       */
    uint8_t magic[6];       /* must be "ustar", NUL-terminated  */
    uint8_t version[2];     /* must be "00" (zero-zero)         */
    uint8_t uname[32];      /* NUL-terminated                   */
    uint8_t gname[32];      /* NUL-terminated                   */
    uint8_t devmajor[8];    
    uint8_t devminor[8];
    uint8_t prefix[155];    /* NUL-terminated if NUL fits       */      
};

#endif