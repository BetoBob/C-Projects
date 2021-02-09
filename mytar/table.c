#include "mytar.h"

void print_verbose(header * head, char * path)
{
    mode_t mod = strtol(head->mode, NULL, 8);

    time_t time_val = strtol(head->mtime, NULL, 8);
    struct tm * local = localtime(&time_val);

    char * names = safe_malloc(17 * sizeof(char));
    char * time_string = safe_malloc(18 * sizeof(char));

    int size = strtol(head->size, NULL, 8);
    int time_temp = strtol(head->mtime, NULL, 8);
    
    /* type of file */
    switch (*head->typeflag) 
    {
        case '5':  putchar('d'); break; /* directory */
        case '2':  putchar('l'); break; /* symbolic link */
        case '0':  putchar('-'); break; /* regular file */
        default:   putchar('-'); break;
    }    
    
    /* permissions */
    putchar( (mod & S_IRUSR) ? 'r' : '-');
    putchar( (mod & S_IWUSR) ? 'w' : '-');
    putchar( (mod & S_IXUSR) ? 'x' : '-');
    putchar( (mod & S_IRGRP) ? 'r' : '-');
    putchar( (mod & S_IWGRP) ? 'w' : '-');
    putchar( (mod & S_IXGRP) ? 'x' : '-');
    putchar( (mod & S_IROTH) ? 'r' : '-');
    putchar( (mod & S_IWOTH) ? 'w' : '-');
    putchar( (mod & S_IXOTH) ? 'x' : '-');
    putchar(' ');

    strtol(head->uname, NULL, 8);

    strcpy(names, head->uname);
    names[strlen(names)] = '/';
    strcat(names, head->gname);

    printf("%-17s", names);
    
    printf("%8d ", size);

    strftime(time_string, 18, "%Y-%m-%d %H:%M ", local);
    printf(time_string);

    printf("%s\n", path);

    /* free stuff */
    free(names);
    free(time_string);

}

/* print the paths of the selected
files/ directories within a tar file */
void table(int v, int s, char *argv[], int argc)
{
    int infd;

    int i, file_index, size;

    header * head;
    uint8_t flag, * path;

    char c = 1; /* checks for end of file */

    int h_lens[] = { HEADER_LENGTHS };
    int h_off[] = { HEADER_OFFSETS };
    path = safe_malloc(sizeof(uint8_t) * PATHMAX);

    head = safe_malloc(sizeof(header));

    /* opens a file for reading only */
    if ( (infd = open(argv[2], O_RDONLY)) < 0)
    {
        perror(argv[2]);
        exit(-1);
    }
    
    while((c != EOF) && (c != 0))
    {
        /* build the header */
        for(i=0; i < HEADER_COUNT; i++)
            read_header(infd, sizeof(uint8_t) * h_lens[i], \
            (uint8_t*) (head->name + h_off[i]));

        /* bogus checker */
        if(strncmp(head->magic, "ustar", 5))
        {
            fprintf(stderr, BOGUS);
            exit(-1);
        }

        /* go to end of header */
        lseek(infd, 12, SEEK_CUR);

        /* compare using strncmp to obtain 
        all the contents within a certain directory */
        path_concat(head->name, head->prefix, path);
        flag = *head->typeflag;

        if(argc == 3)
            (v) ? print_verbose(head, path) : printf("%s\n", path);
        else
        {
            for(file_index = 3; file_index < argc; file_index++, c = 1)
            {   
                if(strlen(path) >= strlen(argv[file_index]))
                {
                    if(!strncmp(argv[file_index], path, \
                    strlen(argv[file_index])))
                        (v) ? print_verbose(head, path) \
                                : printf("%s\n", path);        
                }
            }
        }
        
        /*  check if it's regular 
            skips contents of file if so */
        if((flag == '0') || (flag == '\0'))
        {
            size = strtol(head->size, NULL, 8);
            lseek(infd, sizeof(uint8_t) * (BLOCK_SIZE * \
            ((size / BLOCK_SIZE) + 1)), SEEK_CUR);
        }

        /* reset the header file object */
        head = safe_realloc(head, sizeof(header));
        path = safe_realloc(path, sizeof(uint8_t) * PATHMAX);
                
        /* check for file end */
        read(infd, &c, sizeof(char));
        lseek(infd, -1, SEEK_CUR);
    }
    
    /* free content, close files */
    free(head);
    free(path);

    /* close the tar file */
    if (close(infd) < 0)
    {
        perror("close fail");
        exit(-1);
    }
    
}