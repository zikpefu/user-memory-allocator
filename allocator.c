#define _GNU_SOURCE

void __attribute__ ((constructor)) start(void);
/*
   Zachary Ikpefua
   ECE 3220
   allocator.c
   project 3
 */
 #include <stdio.h>
 #include <dlfcn.h>
 #include <stdbool.h>
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <string.h>
 #include <math.h>
 #include "list.h"
 #include <stdint.h>
 #include <assert.h>
 #include <stdint.h>

 #define PAGESIZE 4096
 #define MAXSIZE 1024
 #define MINSIZE 2
 #define TOTALENTRIES 11
 #define ADJUST 1//reduce the iterations by 1
 #define CUTOFF 0xfffffffff000
Header headerptrs[TOTALENTRIES] ={NULL};
int fd;
/*
   function createLink
   inputs: new - small list header ptr
   output: none
   purpose: create a newpage from the header pointer included,
        similar to createPage

 */
void createLink(Header new){
        void * page = mmap (NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        int byte_size = new->size;
        new->nextpage = page;
        new->nextpage->size = byte_size;
        new->nextpage->freelist = page + sizeof(header_t);
        new->nextpage->nextpage = NULL;
        void * rover = (void *)new->nextpage->freelist;
        Chunk follower = rover;
        int y = (PAGESIZE - sizeof(header_t)) / (byte_size + sizeof(Chunk));
        for(int i = 0; i < y - ADJUST; i++ ) {
                rover += sizeof(Chunk) + byte_size;
                follower->next = rover;
                follower = follower->next;
        }
}
/* createPage
   Inputs: integer x, the power that should be raised with 2 (2^x) to get he correct size
   Outputs: none
   Purpose: Create a newpage of 4096, create the header, then divide the page up into the given size
 */
void createPage(int x){
        int byte_size = pow(2,x);
        void * page = mmap (NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        headerptrs[x] = page;
        headerptrs[x]->size = byte_size;
        headerptrs[x]->freelist = page + sizeof(header_t);
        headerptrs[x]->nextpage = NULL;
        void * rover = (void *)headerptrs[x]->freelist;
        Chunk follower = rover;
        int y = (PAGESIZE - sizeof(header_t)) / (byte_size + sizeof(Chunk));
        for(int i = 0; i < y - ADJUST; i++) {
                rover += sizeof(Chunk) + byte_size;
                follower->next = rover;
                follower = follower->next;
        }
}
/* giveBlock
   inputs: x reference to the correct header
   output: pointer to the block that needs to be returned
   Purpose: if the freelist is null this function will call createlink to
        create a link between the header and the new header, otherwise
        just change the freelist and se the return pointer to the freelist before the move

 */
void * giveBlock(int x){
//return the head of the free list and move it over by 1
        Header rover = headerptrs[x];
        while(rover->freelist == NULL) {
                /*
                   if freelist is null but next page isnt stop and goto the next page
                   if freelist is null and next page is null stop and create a new page
                 */
                if(rover->nextpage != NULL) {
                        rover = rover->nextpage;
                }
                else{
                        createLink(rover);
                }
        }

        void * ret = rover->freelist;
        rover->freelist = rover->freelist->next;
        return ret;
}
/* compareAddress
   intput: address to a header location
   output: true if it is a small (< 1024) false otherwise

 */
bool compareAddress(void * ptr){
        for(int i = 0; i < TOTALENTRIES; i++) {
                if(headerptrs[i] == NULL) {
                        continue;
                }
                else{
                        if(ptr == (void*)headerptrs[i])
                        {
                                return true;
                        }
                }
        }
        return false;
}
//start
//no inputs or outputs
// used for initializing the memory to zero (fd).
void start(void){
        fd = open("/dev/zero", O_RDWR);
}
/* malloc
   input : size of the needed malloc
   outptut: pointer to malleced memory
   purpose: Segment a part of memory to give to the user
 */
void *  malloc(size_t size){
        int x = ceil(log2(size));
        if(size == 0) {
                return NULL;
        }
        else if(size > MAXSIZE) {
                //give a page
                int giveAmnt = (size + sizeof(Large_block))/ PAGESIZE  + ADJUST;
                void * page = mmap (NULL,  giveAmnt * PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                Large_block ret_ptr = page;
                ret_ptr->byte_size = size;
                return ret_ptr + sizeof(Large_block);
        }
        else{
                if(headerptrs[x] == NULL && size != 0) {
                        //create the one page block and split it into sections
                        if(size < MINSIZE) {
                                x = ADJUST;
                        }
                        createPage(x);
                }
                void * ret_ptr = giveBlock(x);
                return ret_ptr + sizeof(Chunk);
        }
        return NULL;

}
/* free
   inputs: pointer to be freed
   outputs: none
   purpose: put the memory that the user doesent want back to the freelist
 */
void free(void * ptr){
        if(ptr == NULL) {
                //dont do anything
                return;
        }
        else{
                void * page_ptr = (void*)((uintptr_t)ptr & CUTOFF);
                bool is_lower_num = compareAddress(page_ptr);
                if(!is_lower_num) {
                        int given = (((Large_block)page_ptr)->byte_size + sizeof(Large_block))/ PAGESIZE  + ADJUST;
                        munmap(page_ptr, PAGESIZE * given);
                }
                else{
                        ptr = ptr - sizeof(Chunk);
                        ((Chunk)ptr)->next  = ((Header)page_ptr)->freelist;
                        ((Header)page_ptr)->freelist = ptr;
                }
        }
}
/* calloc
   inputs: size of needed block(size ), how many sizes needed (nmemb)
   output: pointer to memory given to user
   purpose: similar to malloc but memory is set to 0 by memset
 */
void * calloc(size_t nmemb, size_t size){
        if(nmemb == 0 || size == 0) {
                return NULL;
        }
        else{
                void * ptr = malloc(nmemb * size);
                memset(ptr, 0, nmemb * size);
                return ptr;
        }
}
/* realloc
   inputs: pointer to be changed, size to be changed to
   output: new pointer to the user
   purpose:  Function will change the ptr to the size that the user requests
        if a smaller size is requested the pointer is returned
 */
void * realloc(void * ptr, size_t size){

//memcopy if the size is larger
        if(ptr == NULL) {
                return malloc(size);
        }
        else if(size == 0 && ptr != NULL) {
                free(ptr);
                return NULL;
        }
        else{
                void * page_ptr = (void*)((uintptr_t)ptr & CUTOFF);
                bool is_lower_num = compareAddress(page_ptr);
                if(!is_lower_num) {
                        if(((Large_block)page_ptr)->byte_size > size) {
                                return ptr;
                        }
                        else{
                                void * new_ptr = malloc(size);
                                memcpy(new_ptr,ptr,((Large_block)page_ptr)->byte_size);
                                free(ptr);
                                return new_ptr;
                        }
                }
                else{
                        if (((Header)page_ptr)->size > size) {
                                return ptr;
                        }
                        else{
                                void * new_ptr = malloc(size);
                                memcpy(new_ptr,ptr, ((Header)page_ptr)->size);
                                free(ptr);
                                return new_ptr;
                        }
                }
        }

}
