#include "huffman.h"

#define FUNCTION "./cp_unbuff"

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

tnode * find_alpha(tnode * head, u_int32_t c)
{
    if(head == NULL)
    {
        printf("Error: could not find letter in Huffman Tree");
        exit(-1);
    }
    if(head->ASCII == c)
    {
        return head;
    }
    return find_alpha(head->alpha_link, c);
}

int main(int argc, char * argv[])
{
    int infd, outfd;
    int num, i, bc = 0;

    int hash[HASH_SIZE];

    tnode * t_list = NULL, * temp = NULL, * alpha_head = NULL, * huff = NULL;

    u_int32_t * tnc; /* t_node count */
    u_int8_t * buff, * init, mask = 0x80;   /* u_int8_t buff for writing hex */

    tnc = (u_int32_t*)safe_malloc(sizeof(u_int32_t));
    buff = (u_int8_t*)safe_malloc(sizeof(u_int8_t));
    init = (u_int8_t*)safe_malloc(sizeof(u_int8_t));

    if(argc != 3)
    {
        printf("Invalid number of arguments\n");
        exit(-1);
    }

    /* initializes hash array contents to zero */
    for (i=0; i < HASH_SIZE; i++) { hash[i] = 0; }

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

    while((num = read(infd, buff, sizeof(char))) > 0)
    {
        hash[(int)*buff] += 1;
    }

    /* creates hashnodes for each hash index */
    for (i=0; i < HASH_SIZE; i++) 
    {
        if (hash[i] > 0) 
        {
            t_list = insert_sort_alpha(t_list, create_tnode(i, hash[i]));
            (*tnc)++;
        }
    }

    temp = t_list;

    while(temp != NULL)
    {
        alpha_head = alpha_linker(alpha_head, temp);
        temp = temp->link;
    }

    if(write(outfd, tnc, sizeof(u_int32_t)) != sizeof(u_int32_t))
    {
        perror("write");
        exit(-1);
    }

    temp = alpha_head;
    
    while(temp != NULL)
    {
        *buff = temp->ASCII;
        *tnc  = temp->freq;
        if(write(outfd, buff, sizeof(u_int8_t)) != sizeof(u_int8_t))
        {
            perror("write");
            exit(-1);
        }
        if(write(outfd, tnc, sizeof(u_int32_t)) != sizeof(u_int32_t))
        {
            perror("write");
            exit(-1);
        }
        temp = temp->alpha_link;
    }

    huff = huff_build(t_list);
    huff_traverse(huff); /* creates paths for nodes (of an unsigned int form) */
    
    lseek(infd, 0, SEEK_SET); /* rewinds the file */
    
    /* traverses the file and encodes outfd with 1's and 0's*/
    *init = 0x00;
    while((num = read(infd, buff, sizeof(u_int8_t))) > 0)
    {
        temp = find_alpha(alpha_head, *buff);
        for(i=0; i<strlen(temp->path); i++)
        {
            if(temp->path[i] == '1')
            {
                *init = *init + mask;
            }
            mask = mask >> 1;
            if((++bc) == 8)
            {
                if(write(outfd, init, sizeof(u_int8_t)) != sizeof(u_int8_t))
                {
                    perror("write");
                    exit(-1);
                }
                *init = 0x00; mask = 0x80;
                bc = 0;
            }
        }
    }

    if((bc != 8) && (bc > 4))
    {
        if(write(outfd, init, sizeof(u_int8_t)) != sizeof(u_int8_t))
        {
            perror("write");
            exit(-1);
        }
    }
    
    /* close files */
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

    free(tnc);
    free(init);
    free(buff);

    dealocate(huff);

    return 0;
}