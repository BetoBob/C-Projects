#include "huffman.h"

#define FUNCTION "./htable"

/* I/O Stuff to Convert to Unbuffered I/O */
void read_in(int argc, char *argv[], int hash[])
{
    FILE *in_fp = NULL;
    int c;

    if (argc != 2)
    {
        exit(-1);
    }    
    else if (!(in_fp = fopen(argv[1], "r")))
    {
        perror(argv[1]);
        exit(-1);
    }
    else
    {
        /* change to unbuffered I/O */
        while ((c = fgetc(in_fp)) != EOF)
        {
            hash[c] += 1;
        }
    }
    fclose(in_fp);
}

/* returns returns a binary Huffman-coded file 
 * that's encoded by it's input */
int main(int argc, char *argv[]) 
{
    int hash[HASH_SIZE], i;
    tnode * head = NULL, * alpha_head = NULL, * temp = NULL, * huff = NULL;

    /* initializes hash array contents to zero */
    for (i=0; i < HASH_SIZE; i++) { hash[i] = 0; }

    read_in(argc, argv, hash); /* Use Unbuffered IO */

    /* creates hashnodes for each hash index */
    for (i=0; i < HASH_SIZE; i++) 
    {
        if (hash[i] > 0) 
        {
            head = insert_sort_alpha(head, create_tnode(i, hash[i]));
        }
    }

    temp = head;

    while(temp != NULL)
    {
        alpha_head = alpha_linker(alpha_head, temp);
        temp = temp->link;
    }

    huff = huff_build(head);
    huff_traverse(huff); /* creates paths for nodes (of an unsigned int form) */
    

    while(alpha_head != NULL)
    {
        temp = alpha_head;
        printf("0x%02x: %s\n", alpha_head->ASCII, alpha_head->path);
        alpha_head = alpha_head->alpha_link;
        free(temp);
    }

    return 0;
}
