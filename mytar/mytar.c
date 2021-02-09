#include "mytar.h"

#include "create.c"
#include "extract.c"
#include "table.c"

int v = 0; /* verbose mode */
int s = 0; /* strict mode */
int f = 0; /* specific archive mode */

int lookForFlags(char * flags, char * name)
{
    int i, len;
    int count = 0, invalid = 0;
    int c = 0;

    len = strlen(flags);

    for (i = 0; i < len; i++)
    {
        if(flags[i] == 'c') { c = 'c'; count++; }
        else if(flags[i] == 't') { c = 't'; count++; }
        else if(flags[i] == 'x') { c = 'x'; count++; }
        else if(flags[i] == 'v') { v = 1  ;          }
        else if(flags[i] == 's') { s = 1  ;          }
        else if(flags[i] == 'f') { f = 1  ;          }
        else
        {
            invalid = 1; 
            printf("%s: unrecognized option '%c'.\n", name, flags[i]);
        }
    }

    if(invalid == 1)
    {
        if(count == 0)
        {
            fprintf(stderr, "%s: %s%s", name, \
            NO_ARGS_MESSAGE, USAGE_MESSAGE);
            exit(-1);
        }
        fprintf(stderr, USAGE_MESSAGE);
        exit(-1);
    }
    if(count == 0)
    {
        fprintf(stderr, "%s: %s", name, NO_ARGS_MESSAGE);
        exit(-1);
    }
    if(count > 1)
    {
        fprintf(stderr, "%s: %s", name, TOO_MANY_OPTS_MESSAGE);
        exit(-1);
    }
    return c;
}

int main (int argc, char *argv[])
{
    char * name = argv[0];
    char * flags = argv[1];

    int opt;

    /* check for correct number of options */
    if(argc < 2)
    {
        fprintf(stderr, "%s%s", NO_ARGS_MESSAGE, USAGE_MESSAGE);
        exit(-1);
    }

    /* look for options */
    opt = lookForFlags(flags, name);
    
    if(f == 1)
    {
        /* no arguments given for archive name */
        if(argc == 2)
        {
            fprintf(stderr, "%s: %s%s", name, F_MESSAGE, USAGE_MESSAGE);
            exit(-1);
        }
        if(opt == 'c')
            create(argc, argv, v);
        if(opt == 't')
            table(v, s, argv, argc);
        if(opt == 'x')
            extract(argc, argv);
    }
    else
    {
        /* support for stdin if time permits */
    }
    
    return 0;
}