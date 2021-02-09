#include "mush.h"

stage * create_stage(char * line)
{
    int i;

    stage * cur_stage = (stage*)malloc(sizeof(stage));
    if(cur_stage == NULL)
    {
        perror("stage malloc fail");
        exit(1);
    }
    cur_stage->name = line;
    for(i=0; i < ARGS_MAX; i++) { cur_stage->argv[i] = NULL; }
    return cur_stage;
}

/* checks if a string is empty 
 * (a char other than space exists) */
int is_empty(char * line)
{
    while(*line)
    {
        if(!isspace(*line))
            return 0;
        line++;
    }
    return 1;
}

/* splits pipes and puts each command
 * into an array of char pointers */
int pipe_split(char * line, pipeline * pl)
{
    int i;
    char * tok; /* str token */

    tok = strtok(line, "|"); /* setup pipe token */

    /* split into individual stages */
    for (i = 0; tok != NULL; i++, tok = strtok(NULL, "|"))
    {
        /* check if PIPE_MAX has been reached */
        if(i == PIPE_MAX)
        {
            fprintf(stderr, "pipeline too deep\n");
            return -1;
        }
        /* check if stage is NULL */
        if(is_empty(tok))
        {
            fprintf(stderr, "invalid null line\n");
            return -1;
        }
        pl->stgv[i] = create_stage(tok);
    }
    return i;
}

/* splits command line arguments by space
 * and puts them into a char array */
int args_split(pipeline * pl, int pos)
{
    int s = 0;
    int in_bool = 0, out_bool = 0;
    
    char * space;
    char * line = pl->stgv[pos]->name;

    while (line != NULL)
    {
        if(*line == '<')
        {
            if(pos != 0)
            {
                fprintf(stderr, "ambiguous input\n");
                return 1; 
            }
            if(in_bool > 1)
            {
                fprintf(stderr, "bad input redirection\n");
                return 1; 
            }
            in_bool = 1;
        }
        else if(*line == '>')
        {
            if(pos != (pl->stgc - 1))
            {
                fprintf(stderr, "ambiguous output\n");
                return 1; 
            }
            if(out_bool > 1)
            {
                fprintf(stderr, "bad input redirection\n");
                return 1; 
            }
            out_bool = 1;
        }
        else if(in_bool == 1)  
        {
            pl->input = line;
            in_bool += 1;
        }
        else if(out_bool == 1)
        {
            pl->output = line; 
            out_bool += 1;
        }
        else if(!isspace(*line) && (*line))
            pl->stgv[pos]->argv[s++] = line;
        
        /* Exceeds Max Arguments */
        if(s == (ARGS_MAX + 1))
        {
            fprintf(stderr, "Cannot exceed %d arguments\n", ARGS_MAX);
            return 1; /* graceful exit */
        }

        space = strchr(line, ' ');
        if(space == 0)
            break;

        line[space - line] = '\0';
        line = space + 1;
    }

    pl->stgv[pos]->argc = s;

    return 0;
}

int parseline(char * line, pipeline * pl)
{
    int i;

    /* split into individual stages */
    pl->stgc = pipe_split(line, pl);
    if(pl->stgc == -1)
        return 1;

    for(i = 0; i < pl->stgc; i++)
    {
        if(args_split(pl, i))
            return 1;
    }

    return 0;

}
