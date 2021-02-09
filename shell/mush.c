#include "mush.h"
#include "parseline.c"

static int stgc = 0;
static pid_t child[PIPE_MAX]; 
struct sigaction sa, old;

void interactive_mode(void);

static void kill_children(int signum)
{
    int i;

    putchar('\n'); fflush(stdout);

    for (i = 0; i < stgc; ++i)
    {
        if (child[i] > 0)
        {
            kill(child [i], SIGINT);
            waitpid(child [i], 0, 0);
        }
    }

    /* back to original handler */
    if(-1 == sigaction(SIGINT, &sa, &old))
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void mush_cd(stage * stg)
{
    if(stg->argc != 2)
    {
        fprintf(stderr, "cd: invalid number of arguments\n");
        fflush(stdout);
    }
    if((chdir(stg->argv[1])) < 0)
    {
        perror(stg->argv[1]);
        fflush(stdout);
    }
}

void exec_commands(pipeline * pl)
{
    int i;
    int user_in, user_out;
    int old[2], next[2];    /* the pipes */

    stgc = pl->stgc;

    if(pipe(old))
    {
        perror("old pipe");
        return;
    }

    for(i=0; i < pl->stgc; i++)
    {
        /* create new pipe if not at end of pipeline */
        if(i < pl->stgc - 1)
        {
            /* create a new pipe */
            if(pipe(next))
            {
                perror("next pipe");
                return;
            }
        }

        /* catch if change directory */
        if(!strcmp(pl->stgv[i]->argv[0], "cd"))
        {
            mush_cd(pl->stgv[i]);
            stgc--;
        }
        else if( !(child[i] = fork()))
        {
            /* child process */

            /* check for user-specified STDIN */
            if((i == 0) && (pl->input != NULL))
            {
                if((user_in = open(pl->input, O_RDONLY)) < 0)
                {
                    perror(pl->input);
                    exit(EXIT_FAILURE);
                }
                if(-1 == dup2(user_in, STDIN_FILENO))
                {
                    perror("dup2 old read");
                    exit(EXIT_FAILURE);
                }
                if(-1 == close(user_in))
                {
                    perror("close fail");
                    exit(EXIT_FAILURE);
                }
            }
            else if(-1 == dup2(old[READ_END], STDIN_FILENO))
            {
                perror("dup2 old read");
                exit(EXIT_FAILURE);
            }

            /* writing to the next pipe */
            if(i < pl->stgc-1)
            {
                if(-1 == dup2(next[WRITE_END], STDOUT_FILENO))
                {
                    perror("dup2 new write");
                    exit(EXIT_FAILURE);
                }
            }
            else if(pl->output != NULL) /* check for user-specified STDOUT */
            {
                if((user_out = open(pl->output, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
                {
                    perror(pl->input);
                    exit(EXIT_FAILURE);
                }
                if(-1 == dup2(user_out, STDOUT_FILENO))
                {
                    perror("dup2 new write");
                    exit(EXIT_FAILURE);
                }
                if(-1 == close(user_out))
                {
                    perror("close fail");
                    exit(EXIT_FAILURE);
                }
            }

            /* close all pipes */
            close(old[0]); close(old[1]);
            close(next[0]); close(next[1]);
            
            /* exec command */
            execvp(pl->stgv[i]->argv[0], pl->stgv[i]->argv);
            perror(pl->stgv[i]->argv[0]);
            fflush(stdout);
            exit(EXIT_FAILURE); 
        }

        /* close up old pipe */
        close(old[0]); close(old[1]);

        /* move file descriptors to next pipe */
        old[0] = next[0]; old[1] = next[1];

    }

    /* wait for all child process to complete! */
    while(stgc-- > 0)
    {
        if(-1 == wait(NULL))
            perror("wait");
    }

}

pipeline * create_pipeline(void)
{
    pipeline * pl = malloc(sizeof(pipeline));
    if(pl == NULL)
    {
        perror("pipeline malloc");
        exit(EXIT_FAILURE);
    }
   
    pl->stgc = 0;

    return pl;
}

void free_pipeline(pipeline * pl)
{
    int i;

    for(i = 0; i < pl->stgc; i++)
    {
        if(pl->stgv[i] != NULL)
            free(pl->stgv[i]);
    }

    pl->input = NULL;
    pl->output = NULL;

    free(pl);
}

void interactive_mode(void)
{
    char line[PIPELINE_MAX];
    pipeline * pl;

    /* scan in the command line argument */
    if(isatty(fileno(stdin)))
    {
        printf("8-P ");
        fflush(stdout);
    }

    //scanf(" %[^\n]s", line);
    fgets(line, PIPELINE_MAX, stdin);
    line[strlen(line) - 1] = '\0';

    if(!isatty(fileno(stdin)))
    {
        if(feof(stdin))
            exit(EXIT_SUCCESS);
    }

    if (strlen(line) > PIPELINE_MAX)
        fprintf(stderr, "line too long\n");
    else 
    {

        pl = create_pipeline();
    
        if(!parseline(line, pl))
            exec_commands(pl);

        free_pipeline(pl);

    }
}

void file_mode(char * file_name)
{
    char * line = NULL;
    size_t len = PIPE_MAX;
    int check = 0;
    FILE *fp = NULL;
    pipeline * pl;

    if(NULL == (fp = fopen(file_name, "r"))) 
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while((check = getline(&line, &len, fp)) != -1) {
        if(check > PIPELINE_MAX)
            fprintf(stderr, "line too long\n");
        else
        {
            line[check - 1] = '\0';

            pl = create_pipeline();

            if(!parseline(line, pl))
                exec_commands(pl);

            free_pipeline(pl);
        }
    }

    if(-1 == fclose(fp))
    {
        perror("close fail");
        exit(EXIT_FAILURE);
    }

    free(line);
    exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[])
{

    /* signal stuff here */
    sa.sa_handler = kill_children;      /* set up the handler */
    sigemptyset(&sa.sa_mask);           /* mask nothing */
    sa.sa_flags = 0;                    /* no special handling */

    /* signal handler for cntrl-c */
    if(-1 == sigaction(SIGINT, &sa, &old))
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if(argc > 2)
    {
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }

    /* read commands from a file */
    if(argc == 2)
        file_mode(argv[1]);

    for(;;)
        interactive_mode();

    return EXIT_FAILURE;
}
