
/*********************************************************
 *This file is the header file for the my_malloc library.*
 *********************************************************/

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Block Definition
struct FREE_BLOCK {
    //METADATA
    size_t size;
    struct FREE_BLOCK * next;
    struct FREE_BLOCK * prev;
} typedef FREE_BLOCK;

//First Fit malloc/free
void * ff_malloc(size_t size);
void ff_free(void * pointer);

//Best Fit malloc/free
void * bf_malloc(size_t size);
void bf_free(void * pointer);

//Metrics
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes