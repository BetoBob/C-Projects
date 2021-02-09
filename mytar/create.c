#include "mytar.h"
#include "header.c"

void add_file(int tarfile, char * abs_path, char * rel_path);

/*  recursively calls the 
    add_file function within a directory */
void add_dir_contents(int tarfile, char * abs_path, char * rel_path)
{
    DIR * dir;
    struct dirent * file;
    char * dir_path;
    char * open_path;

    dir_path = safe_malloc(sizeof(char) * PATHMAX);
    strcpy(dir_path, rel_path);
    open_path = safe_malloc(sizeof(char) * PATHMAX);
    strcpy(dir_path, rel_path);

    if(strcpy(open_path, abs_path) < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if(strcat(open_path, "/") < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if(strcat(open_path, rel_path) < 0)
    {
        perror("directory too long");
        exit(-1);
    }

    if (NULL != (dir = opendir(open_path)))
    {
        chdir(rel_path);
        while ((file = readdir(dir)) != NULL)
        {
            dir_path = safe_malloc(sizeof(char) * PATHMAX);
            strcpy(dir_path, rel_path);
            if(strcat(dir_path, "/") < 0)
            {
                perror("directory too long");
                exit(-1);
            }
            if(strcat(dir_path, file -> d_name) < 0)
            {
                perror("directory too long");
                exit(-1);
            }
            if ((strcmp(file -> d_name, "..") != 0) && \
            (strcmp(file -> d_name, ".") != 0))
                add_file(tarfile, abs_path, dir_path);
            free(dir_path);
        }
        closedir(dir);
        chdir("..");
    }
    else
    {
        perror("opendir fail");
    }
}

/* for regular files */
void add_reg(char * abs_path, char * rel_path, int tarfile, int size)
{
    int infd, i, null_space;
    uint8_t * int_buff;
    char * open_path;
    int_buff = safe_malloc(sizeof(uint8_t) * BLOCK_SIZE);
    open_path = safe_malloc(sizeof(char) * PATHMAX);

    /* open regular file for reading */
    if(strcpy(open_path, abs_path) < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if(strcat(open_path, "/") < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if(strcat(open_path, rel_path) < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if ((infd = open(open_path, O_RDONLY)) < 0)
    {
        perror("open fail");
        exit(-1);
    }
    
    for(; size > BLOCK_SIZE; size -= BLOCK_SIZE)
    {
        /* read file contents */
        if(read(infd, int_buff, BLOCK_SIZE * sizeof(uint8_t)) < 0)
        {
            perror("read error");
            exit(-1);
        } 

        /* write to new file */
        if(write(tarfile, int_buff, BLOCK_SIZE * sizeof(uint8_t)) \
        != (BLOCK_SIZE * sizeof(uint8_t)))
        {
            perror("write error");
            exit(-1);
        }
    }

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
        if(write(tarfile, int_buff, sizeof(uint8_t)) \
        != (sizeof(uint8_t)))
        {
            perror("write");
            exit(-1);
        }
    }
    
    *int_buff = 0;
    
    lseek(tarfile, null_space - 1, SEEK_CUR);

    if(write(tarfile, int_buff, sizeof(uint8_t)) \
        != (sizeof(uint8_t)))
    {
        perror("write error");
        exit(-1);
    }
    
    free(int_buff);
    free(open_path);

    if (close(infd) < 0)
    {
        perror("close fail");
        exit(-1);
    }
}

void add_file(int tarfile, char * abs_path, char * rel_path)
{
    struct stat sbuff;
    header head;
    char * stat_path;
    int c, j;

    /* zeros out the header space */
    memset((uint8_t *) (&head), '\0', BLOCK_SIZE);

    /* Set Magic, Null Terminate Version */
    strcpy(head.magic, "ustar");
    head.version[0] = '0'; head.version[1] = '0';
    
    /* getting absolute path for stat function */
    stat_path = safe_malloc(sizeof(char) * PATHMAX);
    strcpy(stat_path, abs_path);
    if(strcat(stat_path, "/") < 0)
    {
        perror("directory too long");
        exit(-1);
    }
    if(strcat(stat_path, rel_path) < 0)
    {
        perror("directory too long");
        exit(-1);
    }

    if (lstat(stat_path, &sbuff) == 0)
    {
        headerName(rel_path, &head, sbuff);
        headerMode(sbuff, head.mode);
        headerUID(sbuff, head.uid);
        headerGID(sbuff, head.gid);
        headerSize(sbuff, head.size);
        headerMTime(sbuff, head.mtime);
        headerTypeFlag(sbuff, head.typeflag);
        headerLinkName(sbuff, rel_path, head.linkname);
        headerUname(sbuff, head.uname);
        headerGname(sbuff, head.gname);
        headerDevmajor(sbuff, head.devmajor);
        headerDevminor(sbuff, head.devminor);
        headerChkSum(&head, head.chksum);

        /* write the contents of the header */
        if (-1 == write(tarfile, (uint8_t *) (&head), BLOCK_SIZE))
        {   
            perror("write header fail");
            exit(-1);
        }

        if (S_ISDIR(sbuff.st_mode))
            add_dir_contents(tarfile, abs_path, rel_path);

        if (S_ISREG(sbuff.st_mode))
            add_reg(abs_path, rel_path, tarfile, (int)sbuff.st_size);
    }
    else
    {
        perror("stat fail");
    }
}

void pad_end(int tarfile)
{
    uint8_t block[PAD_SIZE];
    memset((uint8_t *) (&block), '\0', PAD_SIZE);
    if(write(tarfile, block, sizeof(uint8_t) * PAD_SIZE) \
        != (sizeof(uint8_t) * PAD_SIZE))
    {
        perror("write");
        exit(-1);
    }
}

void create(int argc, char * argv[], int v)
{
    int i, tarfile;
    char * abs_path;
    char * rel_path;

    if ((tarfile = open(argv[2], (O_WRONLY | O_TRUNC | O_CREAT), \
     (S_IRUSR | S_IWUSR))) < 0)
    {
        perror("open fail");
        exit(-1);
    }

    for (i = 3; i < argc; i++)
    {
        /* malloc relative and absolute paths */
        rel_path = safe_malloc(sizeof(char) * PATHMAX);
        strcpy(rel_path, argv[i]);
        abs_path = safe_malloc(sizeof(char) * PATH_MAX);
        realpath(".", abs_path);

        add_file(tarfile, abs_path, rel_path);
        if (v)
            printf("%s\n", rel_path); /* prints the name of the file */

        free(rel_path);
        free(abs_path);
    }

    pad_end(tarfile);

    if ((close(tarfile)) < 0)
    {
        perror("close fail");
        exit(-1);
    }
}
