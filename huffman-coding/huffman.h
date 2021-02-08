#ifndef HUFFMANH
#define HUFFMANH

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

#define HASH_SIZE 256
#define LEFT '0'
#define RIGHT '1'

typedef struct tnode_st tnode;

struct tnode_st {
    u_int8_t ASCII;             /* int ASCII value */
    u_int32_t freq;             /* number of time ASCII value appears */
    char path[HASH_SIZE - 1];   /* path of 0's and 1's in binary indicating it's position in the tree */
    tnode * left;               /* the left element in the tree */
    tnode * right;              /* the right element in the tree */
    tnode * link;               /* link to next element in sorted linked list (for building) */
    tnode * alpha_link;         /* link to next element in sorted linked list (for aft9er building) */
};

/* for intial testing purposes only */
void print_results(tnode * tn)
{
    while(tn != NULL)
    {
        printf("    Char: %c freq: %d\n", (char)tn->ASCII, tn->freq);
        tn = tn->link;
    }    
    printf("end\n");
}

void append(char* s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
}

tnode * create_tnode(int ASCII, int freq) 
{
    tnode * new_tnode = malloc(sizeof(tnode));
    if (new_tnode == NULL) 
    {
        exit(-1);
    }
    new_tnode -> ASCII = ASCII;
    new_tnode -> freq = freq;
    new_tnode -> left = NULL;
    new_tnode -> right = NULL;
    new_tnode -> link = NULL;
    new_tnode -> alpha_link = NULL;
    new_tnode -> path[0] = '\0';
    return new_tnode;
}

tnode * insert_sort_alpha(tnode * head, tnode * new) 
{
    if (head == NULL) 
    {
        return new;
    }
    else if ((head->freq) > (new->freq))
    {
        new -> link = head;
        return new;
    }
    else if ((head->freq) == (new->freq))
    {
        if ((head->ASCII) > (new->ASCII))
        {
            new -> link = head;
            return new;
        }
        else
        {
            head -> link = insert_sort_alpha(head->link, new);
            return head;
        }
    }
    else 
    {
        head -> link = insert_sort_alpha(head->link, new);
        return head;
    }
}

tnode * insert_sort(tnode * head, tnode * new) 
{
    if (head == NULL) 
    {
        return new;
    }
    else if ((head->freq) >= (new->freq))
    {
        new -> link = head;
        return new;
    }
    else 
    {
        head -> link = insert_sort(head->link, new);
        return head;
    }
}

tnode *  alpha_linker(tnode * temp, tnode * new)
{
    if (temp == NULL) 
    {
        return new;
    }
    else if ((temp->ASCII) > (new->ASCII))
    {
        new -> alpha_link = temp;
        return new;
    }
    else 
    {
        temp -> alpha_link = alpha_linker(temp->alpha_link, new);
        return temp;
    }
}

tnode * huff_build(tnode * head) 
{
    tnode * tn_combo;
    tnode * tn_left;
    tnode * tn_right;

    if (head == NULL) 
    {
        exit(-1);
    }

    if (head -> link == NULL)
    {
        return head;
    }

    tn_left = head;
    tn_right = head->link;
    head = head->link->link;

    tn_combo = create_tnode(0, tn_left->freq + tn_right->freq);
    tn_combo->left = tn_left;
    tn_combo->right = tn_right;
    return huff_build(insert_sort(head, tn_combo));
}

void huff_traverse(tnode * head)
{
    if(head == NULL)
    {
        exit(-1);
    }

    /* left traversal */
    if(head->left != NULL)
    {
        strcpy(head->left->path, head->path);
        append(head->left->path, '0');
        huff_traverse(head->left);
    }

    /* right traversal */
    if(head->right != NULL)
    {
        strcpy(head->right->path, head->path);
        append(head->right->path, '1');
        huff_traverse(head->right);
    }
}

void dealocate(tnode * huff)
{
    
    if(huff->left != NULL)
    {
        dealocate(huff->left);
    }
    if(huff->right != NULL)
    {
        dealocate(huff->right);
    }
    
    free(huff);

}

#endif