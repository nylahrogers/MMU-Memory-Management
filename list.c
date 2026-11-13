
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

/* Allocate a new empty list */
list_t *list_alloc() {
    list_t *l = malloc(sizeof(list_t));

    if (l == NULL) {
        return NULL;
    }

    l->head = NULL;
    return l;
}

/* Allocate a new node */
node_t *node_alloc(block_t *blk) {
    node_t *n = malloc(sizeof(node_t));

    if (n == NULL) {
        return NULL;
    }

    n->blk = blk;
    n->next = NULL;

    return n;
}

/* Free the entire list */
void list_free(list_t *l) {
    if (l == NULL) {
        return;
    }

    node_t *current = l->head;

    while (current != NULL) {
        node_t *next = current->next;

        free(current->blk);
        free(current);

        current = next;
    }

    free(l);
}

/* Print the list */
void list_print(list_t *l) {
    if (l == NULL) {
        return;
    }

    node_t *current = l->head;

    while (current != NULL) {
        printf("PID: %d Start: %d End: %d\n",
               current->blk->pid,
               current->blk->start,
               current->blk->end);

        current = current->next;
    }
}

/* Return the number of nodes */
int list_length(list_t *l) {
    if (l == NULL) {
        return 0;
    }

    int count = 0;
    node_t *current = l->head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

/* Add a block to the back */
void list_add_to_back(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    if (l->head == NULL) {
        l->head = newNode;
        return;
    }

    node_t *current = l->head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
}

/* Add a block to the front */
void list_add_to_front(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    newNode->next = l->head;
    l->head = newNode;
}

/* Add a block at a specific index */
void list_add_at_index(list_t *l, block_t *blk, int index) {
    if (l == NULL || blk == NULL) {
        return;
    }

    if (index <= 0 || l->head == NULL) {
        list_add_to_front(l, blk);
        return;
    }

    node_t *current = l->head;
    int i = 0;

    while (current->next != NULL && i < index - 1) {
        current = current->next;
        i++;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    newNode->next = current->next;
    current->next = newNode;
}

/* Add blocks in ascending order by starting address */
void list_add_ascending_by_address(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    if (l->head == NULL ||
        l->head->blk->start > blk->start) {

        newNode->next = l->head;
        l->head = newNode;
        return;
    }

    node_t *current = l->head;

    while (current->next != NULL &&
           current->next->blk->start <= blk->start) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

/* Add blocks in ascending order by block size (Best Fit) */
void list_add_ascending_by_blocksize(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    int newSize = blk->end - blk->start + 1;

    if (l->head == NULL ||
        (l->head->blk->end - l->head->blk->start + 1) > newSize) {

        newNode->next = l->head;
        l->head = newNode;
        return;
    }

    node_t *current = l->head;

    while (current->next != NULL) {
        int nextSize = current->next->blk->end -
                       current->next->blk->start + 1;

        if (nextSize > newSize) {
            break;
        }

        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

/* Add blocks in descending order by block size (Worst Fit) */
void list_add_descending_by_blocksize(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return;
    }

    node_t *newNode = node_alloc(blk);

    if (newNode == NULL) {
        return;
    }

    int newSize = blk->end - blk->start + 1;

    if (l->head == NULL ||
        (l->head->blk->end - l->head->blk->start + 1) < newSize) {

        newNode->next = l->head;
        l->head = newNode;
        return;
    }

    node_t *current = l->head;

    while (current->next != NULL) {
        int nextSize = current->next->blk->end -
                       current->next->blk->start + 1;

        if (nextSize < newSize) {
            break;
        }

        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
}

/* Remove the last block */
block_t* list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return NULL;
    }

    if (l->head->next == NULL) {
        node_t *temp = l->head;
        block_t *blk = temp->blk;

        l->head = NULL;
        free(temp);

        return blk;
    }

    node_t *current = l->head;

    while (current->next->next != NULL) {
        current = current->next;
    }

    node_t *temp = current->next;
    block_t *blk = temp->blk;

    current->next = NULL;
    free(temp);

    return blk;
}

/* Remove the first block */
block_t* list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return NULL;
    }

    node_t *temp = l->head;
    block_t *blk = temp->blk;

    l->head = temp->next;
    free(temp);

    return blk;
}

/* Remove a block at a specific index */
block_t* list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL || index < 0) {
        return NULL;
    }

    if (index == 0) {
        return list_remove_from_front(l);
    }

    node_t *current = l->head;
    int i = 0;

    while (current->next != NULL && i < index - 1) {
        current = current->next;
        i++;
    }

    if (current->next == NULL) {
        return NULL;
    }

    node_t *temp = current->next;
    block_t *blk = temp->blk;

    current->next = temp->next;
    free(temp);

    return blk;
}

/* Check whether a block exists */
bool list_is_in(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return false;
    }

    node_t *current = l->head;

    while (current != NULL) {
        if (compareBlks(current->blk, blk)) {
            return true;
        }

        current = current->next;
    }

    return false;
}

/* Check whether a block of the requested size or greater exists */
bool list_is_in_by_size(list_t *l, int Size) {
    if (l == NULL) {
        return false;
    }

    node_t *current = l->head;

    while (current != NULL) {
        int blockSize = current->blk->end -
                        current->blk->start + 1;

        if (blockSize >= Size) {
            return true;
        }

        current = current->next;
    }

    return false;
}

/* Check whether a PID exists */
bool list_is_in_by_pid(list_t *l, int pid) {
    if (l == NULL) {
        return false;
    }

    node_t *current = l->head;

    while (current != NULL) {
        if (current->blk->pid == pid) {
            return true;
        }

        current = current->next;
    }

    return false;
}

/* Get a block at a specific index */
block_t* list_get_elem_at(list_t *l, int index) {
    if (l == NULL || index < 0) {
        return NULL;
    }

    node_t *current = l->head;
    int i = 0;

    while (current != NULL) {
        if (i == index) {
            return current->blk;
        }

        current = current->next;
        i++;
    }

    return NULL;
}

/* Get the index of a particular block */
int list_get_index_of(list_t *l, block_t *blk) {
    if (l == NULL || blk == NULL) {
        return -1;
    }

    node_t *current = l->head;
    int index = 0;

    while (current != NULL) {
        if (compareBlks(current->blk, blk)) {
            return index;
        }

        current = current->next;
        index++;
    }

    return -1;
}

/* Get the index of a block with a specific size */
int list_get_index_of_by_Size(list_t *l, int Size) {
    if (l == NULL) {
        return -1;
    }

    node_t *current = l->head;
    int index = 0;

    while (current != NULL) {
        int blockSize = current->blk->end -
                        current->blk->start + 1;

        if (blockSize == Size) {
            return index;
        }

        current = current->next;
        index++;
    }

    return -1;
}

/* Get the index of a block with a specific PID */
int list_get_index_of_by_Pid(list_t *l, int pid) {
    if (l == NULL) {
        return -1;
    }

    node_t *current = l->head;
    int index = 0;

    while (current != NULL) {
        if (current->blk->pid == pid) {
            return index;
        }

        current = current->next;
        index++;
    }

    return -1;
}

/* Return the first block */
block_t* list_get_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return NULL;
    }

    return l->head->blk;
}

/* Compare two memory blocks */
bool compareBlks(block_t *a, block_t *b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    return a->pid == b->pid &&
           a->start == b->start &&
           a->end == b->end;
}

/* Merge physically adjacent free memory blocks */
void list_coalese_nodes(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return;
    }

    /* First sort the free blocks by address */
    node_t *current = l->head;

    while (current != NULL) {
        node_t *smallest = current;
        node_t *search = current->next;

        while (search != NULL) {
            if (search->blk->start < smallest->blk->start) {
                smallest = search;
            }

            search = search->next;
        }

        if (smallest != current) {
            block_t *temp = current->blk;
            current->blk = smallest->blk;
            smallest->blk = temp;
        }

        current = current->next;
    }

    /* Merge adjacent blocks */
    current = l->head;

    while (current != NULL && current->next != NULL) {

        node_t *next = current->next;

        if (current->blk->end != 2147483647 &&
            current->blk->end + 1 == next->blk->start) {

            current->blk->end = next->blk->end;

            node_t *temp = next;
            current->next = next->next;

            free(temp->blk);
            free(temp);

        } else {
            current = current->next;
        }
    }
}
