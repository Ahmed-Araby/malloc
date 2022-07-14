#include <unistd.h>
#include <stdio.h>
#include "malloc.h"

// global variables
void *hb = NULL;
void *mbrk = NULL;
// 


/**
 * @brief will be called only the very first time, 
 * on which the application request to allocate dynamic memory
 */

int hinit(){
    hb = sbrk(0);
    int ret = brk(hb + 4 * WSIZE);
    if(ret != 0){
        printf("error: failed to initialize heap of 4 bytes");
        return -1;
    }
    mbrk = hb + 4 * WSIZE;

    // [--][8/1][8/1][0/1]  (current heap)
    SET(hb, 0);
    SET((hb + WSIZE), 0x9);
    SET((hb + DWSIZE), 0x9);
    SET((hb + 3 * WSIZE), 0x1);
    //
    return 0;
}


/**
 * [TODO] write unit tests
 * private helper methods
 */
void merge(void* leftb, void* rightb){
    SET(BHEADER(leftb), (BSIZE(leftb) + BSIZE(rightb)));
    SET(BFOOTER(rightb), BSIZE(leftb));
}

void coalesce(void* ptr){
    if(BALLOC(ptr) != 0){
        printf("error: heap block pointed to be ptr  = %p should be free", ptr);
        return;
    }

    void* nexthbptr = BNEXT(ptr);
    void* prevhbptr = BPREV(ptr);
    if(BALLOC(nexthbptr) == 0){
        merge(ptr, nexthbptr);
    }
    if(BALLOC(prevhbptr) == 0){
        merge(prevhbptr, ptr);
    }
}

/**
 * @brief extend the heap by EXP_CHUNK(=64byte) at a time
 * 
 * @return int 0 is succeeded, -1 if failed
 */
int extendh(){
    if(mbrk == NULL){
        mbrk = sbrk(0);
    }
    int ret = brk(mbrk + EXP_CHUNK);
    if(ret == 0){
        SET(BHEADER(mbrk), EXP_CHUNK);
        SET(BFOOTER(mbrk), EXP_CHUNK);
        SET((mbrk + EXP_CHUNK - WSIZE), 1); // EPILOGUE block
        coalesce(mbrk);
        mbrk = mbrk + EXP_CHUNK;
        return 0;
    }
    else
        return -1;
}

/**
 * @brief split the heap block pointed to by ptr into two blocks,
 * left block which has payload of size 'size' in bytes and, 
 * right block with has the remaining size (if exist) of the orignal block, 
 * because the original block is dword alligned and 'size' is multiple of dword, 
 * it is garanted that left and right block (if exist) both will be dword alligned.
 * 
 * @param ptr pointer to the payload of the heap block to be splited
 * @param size size of the payload to be allocated in bytes (must be multiple of double word (=8))
 */
void split(void* ptr, unsigned int size) {
    const unsigned int bsize = BSIZE(ptr);
    const unsigned int rbsize  = bsize - size - DWSIZE;
    
    // RIGHT BLOCK
    const void* lh = BHEADER(ptr);
    SET(lh, (size + DWSIZE));
    const void* lf = BFOOTER(ptr);
    SET(lf, (size + DWSIZE));
    //
    // LEFT BLOCK
    const void* rh = lh + size + DWSIZE;
    SET(rh, rbsize);
    const void *rf = BFOOTER((rh + WSIZE));
    SET(rf, rbsize);
    //
}
