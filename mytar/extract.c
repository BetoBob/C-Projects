# include "mytar.h"

void create_reg(int infd, header * head, char * path)
{
    int i, outfd, null_space;

    uint8_t * int_buff;
    struct utimbuf ubuf;
    struct stat sbuf;
    int size, mode;
    
    int_buff = safe_malloc(sizeof(uint8_t) * BLOCK_SIZE);

    /* zero out time buffer for security */
    memset(&ubuf, 0, sizeof(struct utimbuf));

    size = strtol(head->size, NULL, 8);
    mode = strtol(head->mode, NULL, 8); 
    ubuf.modtime = (time_t)strtol(head->mtime, NULL, 8); 

    /* check if file already exists */
    if (!stat(path, &sbuf))
    {
        free(int_buff);
        lseek(infd, sizeof(uint8_t) * (BLOCK_SIZE * \
        ((size / BLOCK_SIZE) + 1)), SEEK_CUR);
        return;
    }
    printf("%s\n", path);

    /* open regular file for writing */
    if ( (outfd = open(path, (O_WRONLY | O_CREAT | O_TRUNC))) < 0)
    {
        perror(path);
        exit(-1);
    }

    /* change this into sizeable chuncks later */
    /* do it in chuncks of 512! */
    for(; size > BLOCK_SIZE; size -= BLOCK_SIZE)
    {
        /* read file contents */
        if(read(infd, int_buff, sizeof(uint8_t) * BLOCK_SIZE) < 0)
        {
            perror("read error");
            exit(-1);
        } 

        /* write to new file */
        if(write(outfd, int_buff, BLOCK_SIZE * sizeof(uint8_t)) \
        != (BLOCK_SIZE * sizeof(uint8_t)))
        {
            perror("write");
            exit(-1);
        }
    }

    /* calculate block space to be padded */
    null_space = sizeof(uint8_t) * (BLOCK_SIZE - size);

    /* write remaining part of File */
    for(i=0; i < size; i++)
    {
        /* read file contents */
        if(read(infd, int_buff, sizeof(uint8_t)) < 0)
        {
            perror("read error");
            exit(-1);
        } 

        /* write to new file */
        if(write(outfd, int_buff, sizeof(uint8_t)) \
        != (sizeof(uint8_t)))
        {
            perror("write");
            exit(-1);
        }
    }
    
    /* change permissions */
    if (chmod(path, mode) < 0)
    {
        perror("chmod fail");
        exit(-1);
    }

    /* change the mod time */
    if(utime(path, &ubuf) < 0)
    {
        perror("utime fail");
        exit(-1);
    }
    
    /* close file being written to */
    if(close(outfd))
    {
        perror("close output file");
        exit(-1);
    }    

    /* go to the end of the block in the file */
    lseek(infd, null_space, SEEK_CUR);

    free(int_buff);
}

void create_dir(header * head, char * path)
{
    struct stat sbuf;
    int mode;

    mode = strtol(head->mode, NULL, 8);

    /* check if file already exists */
    if (stat(path, &sbuf) == 0)
        return;
    
    printf("%s\n", path);

    /* make the new directory */
    mkdir(path, mode);

}

void create_sym(header * head, char * path)
{
    struct stat sbuf;

    /* check if file already exists */
    if (stat(path, &sbuf) == 0)
        return;

    printf("%s\n", path);

    if( symlink(head->linkname, path) < 0)
    {
        perror("symlink creation fail");
        exit(-1);
    }
}

void create_file(int infd, int flag, header * head, char * path)
{
    if((flag == '0') || (flag == '\0'))
        create_reg(infd, head, path);
    else if(flag == '2')
        create_sym(head, path);
    else if(flag == '5')
        create_dir(head, path);
    else
        printf("Invalid file type: %s", head->name);
}

void extract(int argc, char * argv[])
{
    int infd, i, file_index, size, found = 0;

    header * head;
    uint8_t flag, path[PATHMAX];

    char c = 1;

    int h_lens[] = { HEADER_LENGTHS };
    int h_off[] = { HEADER_OFFSETS };

    /* opens a file for reading only */
    if ( (infd = open(argv[2], O_RDONLY)) < 0)
    {
        perror(argv[2]);
        exit(-1);
    }

    head = safe_malloc(sizeof(header));

    if (argc < 3)
    {
        printf("Usage Error");
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
            printf("%s: Not a compatible header\n", head->name);
            exit(-1);
        }

        /* go to end of header */
        lseek(infd, 12, SEEK_CUR);

        path_concat(head->name, head->prefix, path);
        flag = *head->typeflag;

        /* file arguments are given */
        if(argc > 3)
        {
            for(file_index = 3; file_index < argc; file_index++)
            {
                if ((!strncmp(argv[file_index], path, \
                strlen(path)) && (flag == '5')))
                    create_dir(head, path);
                else if(!strncmp(argv[file_index], path, \
                strlen(argv[file_index])))
                {
                    found = 1; /* confirms file/directory in tar file */
                    create_file(infd, flag, head, path);
                }
            }
            if(!found && ((flag == '0') || (flag == '\0')))
            {
                size = strtol(head->size, NULL, 8);
                lseek(infd, sizeof(uint8_t) * (BLOCK_SIZE * \
                ((size / BLOCK_SIZE) + 1)), SEEK_CUR);
            }
        }
        else
        {
            create_file(infd, flag, head, path);
        }

        /* reset the header file object */
        head = safe_realloc(head, sizeof(header));
        found = 0;
            
        /* check for file end */
        read(infd, &c, sizeof(char));
        lseek(infd, -1, SEEK_CUR);
    }

    /* free head, close tarfile */
    free(head);

    if(close(infd))
    {
        perror("close tar file");
        exit(-1);
    }
}
