#include "huffman.h"

#define FUNCTION "./hdecode"

int letter_count = 0; /* total number of letter in a file */

void * safe_malloc(int size) 
{ 
    void *new;
    new = malloc(size);
    if (new == NULL) 
    {
        perror(FUNCTION);
        exit(-1); 
    }
    return new; 
}

void out_write(int outfd, u_int8_t * c)
{
    if(write(outfd, c, sizeof(char)) != sizeof(char))
    {
        perror("write");
        exit(-1);
    }
}

tnode * tree_traverse(tnode * huff, tnode * iter, int bit, int outfd)
{
    /* left traversal */
    if(bit == 1)
    {
        if(iter->right == NULL)
        {
            out_write(outfd, &iter->ASCII);
            --letter_count;
            return huff->right;
        }
        return iter -> right;
    }
    /* right traversal */
    if(iter -> left == NULL)
    {
        out_write(outfd, &iter->ASCII);
        --letter_count;
        return huff -> left;
    }
    return iter -> left;
}

int main(int argc, char * argv[])
{
    int infd, outfd;
    int num, char_count, i, bit;

    char * buff1; 
    u_int32_t * buff2;
    u_int8_t  * buff3, mask = 0x80;

    tnode * huff = NULL, * iter;
    tnode * t_list = NULL;

    buff1 = (char*)safe_malloc(sizeof(char));
    buff2 = (u_int32_t*)safe_malloc(sizeof(u_int32_t)); /* for frequency counts */
    buff3 = (u_int8_t *)safe_malloc(sizeof(u_int8_t));  /* for binary path */

    if(argc != 3)
    {
        printf("Invalid number of arguments\n");
        exit(-1);
    }

    /* opens a file for reading only */
    if ( (infd = open(argv[1], O_RDONLY)) < 0)
    {
        perror(argv[1]);
        exit(-1);
    }
    /* opens a file to write to */
    /* permissions: user read and write only */
    if ( (outfd = open(argv[2],
    (O_WRONLY | O_CREAT | O_TRUNC), (S_IRUSR | S_IWUSR))) < 0)
    {
        perror(argv[2]);
        exit(-1);
    }

    if((num = read(infd, buff2, sizeof(u_int32_t))) < 0)
    {
        perror("read error");
        exit(-1);
    }
    char_count = *buff2;

    /* Read chars and frequencies here */
    for(i = 0; i < char_count; i++)
    {
        if((num = read(infd, buff1, sizeof(char))) < 0)
        {
            perror("read error");
            exit(-1);
        }
        if((num = read(infd, buff2, sizeof(u_int32_t))) < 0)
        {
            perror("read error");
            exit(-1);
        }
        letter_count += *buff2;
        t_list = insert_sort_alpha(t_list, create_tnode(*buff1, *buff2));
    }
    
    huff = huff_build(t_list);
    iter = huff;

    /* catch single leaf trees */
    if(char_count == 1)
    {
        for(i=0; i<letter_count; i++)
        {
            out_write(outfd, &t_list->ASCII);
        }
    }
    else
    {
        /* Read paths here */
        while(((num = read(infd, buff3, sizeof(u_int8_t))) >= 0) && (letter_count > 0))
        {
            for (i=0; i < 8; i++)
            {
                if(letter_count == 0) break;
                bit = (*buff3 & mask);
                if(bit != 0) bit = 1;
                iter = tree_traverse(huff, iter, bit, outfd);
                *buff3 = *buff3 << 1;
            }
            mask = 0x80;
        }
    }

    /* closing files, freeing buffs */
    if(num < 0)
    {
        perror("read");
        exit(-1);
    }
    if(close(infd))
    {
        perror("close");
        exit(-1);
    }
    if(close(outfd))
    {
        perror("close");
        exit(-1);
    }

    free(buff1);
    free(buff2);
    free(buff3);

    dealocate(huff);

    return 0;
}