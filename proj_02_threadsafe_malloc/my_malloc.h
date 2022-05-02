
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

//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size);
void ts_free_lock(void * pointer);

//Thread Safe malloc/free: non-locking version
void * ts_malloc_nolock(size_t size);
void ts_free_nolock(void * pointer);

//Metrics
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes

//Metrics nolock
unsigned long get_data_segment_size_nolock(); //in bytes
unsigned long get_data_segment_free_space_size_nolock(); //in bytes