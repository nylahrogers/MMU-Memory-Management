
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

/* Convert command-line policy to uppercase */
void TOUPPER(char *arr) {
    for (int i = 0; i < strlen(arr); i++) {
        arr[i] = toupper((unsigned char)arr[i]);
    }
}

/* Read input and determine allocation policy */
void get_input(char *args[], int input[][2], int *n,
               int *size, int *policy) {

    FILE *input_file = fopen(args[1], "r");

    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        exit(1);
    }

    parse_file(input_file, input, n, size);
    fclose(input_file);

    TOUPPER(args[2]);

    if (strcmp(args[2], "-F") == 0 ||
        strcmp(args[2], "-FIFO") == 0) {
        *policy = 1;
    }
    else if (strcmp(args[2], "-B") == 0 ||
             strcmp(args[2], "-BESTFIT") == 0) {
        *policy = 2;
    }
    else if (strcmp(args[2], "-W") == 0 ||
             strcmp(args[2], "-WORSTFIT") == 0) {
        *policy = 3;
    }
    else {
        printf("usage: ./mmu <input file> -{F | B | W}\n");
        printf("(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(1);
    }
}

/* Allocate memory to a process */
void allocate_memory(list_t *freelist, list_t *alloclist,
                     int pid, int blocksize, int policy) {

    node_t *current = freelist->head;
    int index = 0;
    int found = 0;

    /* Search for a free block large enough */
    while (current != NULL) {

        int available = current->blk->end -
                        current->blk->start + 1;

        if (available >= blocksize) {
            found = 1;
            break;
        }

        current = current->next;
        index++;
    }

    /* No suitable memory found */
    if (!found) {
        printf("Error: Not Enough Memory\n");
        return;
    }

    /* Remove selected block from free list */
    block_t *blk = list_remove_at_index(freelist, index);

    if (blk == NULL) {
        return;
    }

    /* Save original ending address */
    int original_end = blk->end;

    /* Assign requested memory */
    blk->pid = pid;
    blk->end = blk->start + blocksize - 1;

    /* Add allocated block in address order */
    list_add_ascending_by_address(alloclist, blk);

    /* Create remaining free fragment */
    if (blk->end < original_end) {

        block_t *fragment = malloc(sizeof(block_t));

        if (fragment == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return;
        }

        fragment->pid = 0;
        fragment->start = blk->end + 1;
        fragment->end = original_end;

        /* Insert fragment according to policy */
        if (policy == 1) {
            list_add_to_back(freelist, fragment);
        }
        else if (policy == 2) {
            list_add_ascending_by_blocksize(freelist, fragment);
        }
        else if (policy == 3) {
            list_add_descending_by_blocksize(freelist, fragment);
        }
    }
}

/* Deallocate memory from a process */
void deallocate_memory(list_t *alloclist, list_t *freelist,
                       int pid, int policy) {

    /* Find process in allocated list */
    int index = list_get_index_of_by_Pid(alloclist, pid);

    if (index == -1) {
        printf("Error: Can't locate Memory Used by PID: %d\n", pid);
        return;
    }

    /* Remove allocated block */
    block_t *blk = list_remove_at_index(alloclist, index);

    if (blk == NULL) {
        return;
    }

    /* Mark block as free */
    blk->pid = 0;

    /* Return block according to policy */
    if (policy == 1) {
        list_add_to_back(freelist, blk);
    }
    else if (policy == 2) {
        list_add_ascending_by_blocksize(freelist, blk);
    }
    else if (policy == 3) {
        list_add_descending_by_blocksize(freelist, blk);
    }
}

/* Coalesce physically adjacent free blocks */
list_t* coalese_memory(list_t *list) {

    list_t *temp_list = list_alloc();
    block_t *blk;

    if (temp_list == NULL) {
        return list;
    }

    /* Sort free blocks by address */
    while ((blk = list_remove_from_front(list)) != NULL) {
        list_add_ascending_by_address(temp_list, blk);
    }

    /* Merge adjacent blocks */
    list_coalese_nodes(temp_list);

    /* Release old empty list */
    free(list);

    return temp_list;
}

/* Print memory list */
void print_list(list_t *list, char *message) {

    node_t *current = list->head;
    block_t *blk;
    int i = 0;

    printf("%s:\n", message);

    while (current != NULL) {

        blk = current->blk;

        printf("Block %d:\t START: %d\t END: %d",
               i, blk->start, blk->end);

        if (blk->pid != 0) {
            printf("\t PID: %d\n", blk->pid);
        }
        else {
            printf("\n");
        }

        current = current->next;
        i++;
    }
}

/* DO NOT MODIFY MAIN */
int main(int argc, char *argv[]) {

    int PARTITION_SIZE;
    int inputdata[200][2];
    int N = 0;
    int Memory_Mgt_Policy;

    list_t *FREE_LIST = list_alloc();
    list_t *ALLOC_LIST = list_alloc();

    int i;

    if (argc != 3) {
        printf("usage: ./mmu <input file> -{F | B | W}\n");
        printf("(F=FIFO | B=BESTFIT | W=WORSTFIT)\n");
        exit(1);
    }

    get_input(argv, inputdata, &N,
              &PARTITION_SIZE, &Memory_Mgt_Policy);

    /* Create initial memory partition */
    block_t *partition = malloc(sizeof(block_t));

    if (partition == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    partition->pid = 0;
    partition->start = 0;
    partition->end = PARTITION_SIZE - 1;

    list_add_to_front(FREE_LIST, partition);

    /* Simulate memory management */
    for (i = 0; i < N; i++) {

        printf("************************\n");

        if (inputdata[i][0] != -99999 &&
            inputdata[i][0] > 0) {

            printf("ALLOCATE: %d FROM PID: %d\n",
                   inputdata[i][1], inputdata[i][0]);

            allocate_memory(FREE_LIST, ALLOC_LIST,
                            inputdata[i][0],
                            inputdata[i][1],
                            Memory_Mgt_Policy);
        }

        else if (inputdata[i][0] != -99999 &&
                 inputdata[i][0] < 0) {

            printf("DEALLOCATE MEM: PID %d\n",
                   abs(inputdata[i][0]));

            deallocate_memory(ALLOC_LIST, FREE_LIST,
                              abs(inputdata[i][0]),
                              Memory_Mgt_Policy);
        }

        else {

            printf("COALESCE/COMPACT\n");

            FREE_LIST = coalese_memory(FREE_LIST);
        }

        printf("************************\n");

        print_list(FREE_LIST, "Free Memory");
        print_list(ALLOC_LIST, "\nAllocated Memory");

        printf("\n\n");
    }

    list_free(FREE_LIST);
    list_free(ALLOC_LIST);

    return 0;
}
