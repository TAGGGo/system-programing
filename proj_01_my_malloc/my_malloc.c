
/***********************************************************
 *This file is the implementation of the my_malloc library.*
 ***********************************************************/

#include <stdlib.h>
#include "my_malloc.h"
#include "assert.h"
#include <time.h>

#define METADATA_SIZE sizeof(FREE_BLOCK)

FREE_BLOCK * block_list_head = NULL;
unsigned long block_list_size = 0;
unsigned long data_segment_size = 0;
unsigned long data_segment_free_space_size = 0;

void * malloc_ret_ptr(void * ptr) {
    return ptr + METADATA_SIZE;
}

FREE_BLOCK * to_block_ptr(void * ptr) {
    return ptr - METADATA_SIZE;
}

unsigned long get_data_segment_size() {
    return data_segment_size;
}

unsigned long get_data_segment_free_space_size() {
    return data_segment_free_space_size;
}

FREE_BLOCK * create_block(size_t size) {
    FREE_BLOCK * ptr = sbrk(size + METADATA_SIZE);
    memset(ptr, 0, sizeof(FREE_BLOCK));
    ptr->size = size;
    data_segment_size += (size + METADATA_SIZE);
    return ptr;
}

void emplace_block(FREE_BLOCK * node, FREE_BLOCK * ptr) {
    if(node == NULL) {
        //Add head
        ptr->next = block_list_head;
        ptr->prev = NULL;
        if(block_list_head) {
            block_list_head->prev = ptr;
        }
        block_list_head = ptr;
    }
    else {
        //Add middle
        ptr->next = node->next;
        ptr->prev = node;
        node->next = ptr;
        if(ptr->next) {
            ptr->next->prev = ptr;
        }
    }
    data_segment_free_space_size += (METADATA_SIZE + ptr->size);
    block_list_size++;
}

void delete_block(FREE_BLOCK * ptr) {
    if(ptr == NULL || block_list_size == 0) {
        return;
    }
    if(ptr == block_list_head) {
        block_list_head = ptr->next;
    }
    if(ptr->prev) {
        ptr->prev->next = ptr->next;
    }
    if(ptr->next) {
        ptr->next->prev = ptr->prev;
    }
    data_segment_free_space_size -= (METADATA_SIZE + ptr->size);
    block_list_size--;
}

void merge_block(FREE_BLOCK * begin) {
    FREE_BLOCK * ptr = begin;
    while(ptr) {
        if(ptr->next && ((unsigned long)ptr + METADATA_SIZE + ptr->size) == (unsigned long)ptr->next) {
            //Find continuous blocks, merge them
            ptr->size += (METADATA_SIZE + ptr->next->size);
            ptr->next = ptr->next->next;
            if(ptr->next) {
                ptr->next->prev = ptr;
            }
            block_list_size--;
        }
        else {
            break;
        }
    }
}

FREE_BLOCK * split_block(FREE_BLOCK * ptr, size_t size) {
    FREE_BLOCK * prev = ptr->prev;
    delete_block(ptr);
    if(ptr->size - size <= METADATA_SIZE) {
        return ptr;
    }
    //Split into two blocks
    FREE_BLOCK * next_block = (void *)ptr + METADATA_SIZE + size;
    next_block->size = ptr->size - size - METADATA_SIZE;
    ptr->size = size;
    emplace_block(prev, next_block);
    return next_block;
}

void * ff_malloc(size_t size) {
    FREE_BLOCK * ptr = block_list_head;
    while(ptr) {
        if(ptr->size >= size) {
            //Find first fit block, split block
            split_block(ptr, size);
            return malloc_ret_ptr(ptr);
        }
        ptr = ptr->next;
    }
    //Not found, return new block
    FREE_BLOCK * new_block = create_block(size);
    return malloc_ret_ptr(new_block);
}

void ff_free(void * pointer) {
    FREE_BLOCK * block_ptr = to_block_ptr(pointer);
    FREE_BLOCK * ptr = NULL;
    FREE_BLOCK * next_ptr = block_list_head;
    //Search for position to insert pointer(to be freed)
    while(next_ptr) {
        if((unsigned long)next_ptr > (unsigned long)block_ptr) {
            break;
        }
        ptr = next_ptr;
        next_ptr = next_ptr->next;
    }
    emplace_block(ptr, block_ptr);
    if(ptr == NULL) {
        merge_block(block_list_head);
    }
    else {
        merge_block(ptr);
    }
}

void * bf_malloc(size_t size) {
    FREE_BLOCK * ptr = block_list_head;
    FREE_BLOCK * bestfit_block = NULL;
    size_t min_margin = ULONG_MAX;
    while(ptr) {
        if(ptr->size == size) {
            //Already Find the Best
            delete_block(ptr);
            return malloc_ret_ptr(ptr);
        }
        if(ptr->size >= size && ((ptr->size - size) < min_margin)) {
            min_margin = ptr->size - size;
            bestfit_block = ptr;
        }
        ptr = ptr->next;
    }
    if(bestfit_block) {
        //Found best block
        split_block(bestfit_block, size);
        return malloc_ret_ptr(bestfit_block);
    }
    //Not found, return new block
    FREE_BLOCK * new_block = create_block(size);
    return malloc_ret_ptr(new_block);
}

void bf_free(void * pointer) {
    ff_free(pointer);
}