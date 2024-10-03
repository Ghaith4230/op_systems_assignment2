#include <stdint.h>
#include "mm.h"

typedef struct header {
    struct header *next;
    uint64_t user_block[0];
} BlockHeader;

#define GET_NEXT(p)    (void *) ((uintptr_t)(p->next) & ~0x1)
#define SET_NEXT(p,n)  p->next = (void *) (n & ~0x1)
#define GET_FREE(p)    (uint8_t) ((uintptr_t)(p->next) & 0x1)
#define SET_FREE(p, f) \
    do { \
        if (((uintptr_t)(p->next) & 0x1) != (f)) { \
            p->next = (void *)((uintptr_t)(p->next) ^ 0x1); \
        } \
    } while (0)
#define SIZE(p)        (size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)p - sizeof(BlockHeader))
#define MIN_SIZE     (8)

static BlockHeader *first = NULL;
static BlockHeader *current = NULL;
static  BlockHeader *last = NULL;

void simple_init() {

    uintptr_t aligned_memory_start = memory_start;
    uintptr_t aligned_memory_end = memory_end;


    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            current = (BlockHeader *)(aligned_memory_start + sizeof(BlockHeader));
            first = (BlockHeader *)aligned_memory_start;
            first ->next = current;
            last = (BlockHeader *) memory_end;
            last->next = first;
            current ->next = last;
        }
    }
}


void* simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    size_t aligned_size = (size + 7) & ~0x7;;

    BlockHeader *search_start = current;
    do {
        if (GET_FREE(current)) {
            if (SIZE(current) >= aligned_size) {
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    SET_FREE(current,1);
                } else {

                    current->next = (BlockHeader *)((char *) current + aligned_size);

                }
                BlockHeader * returnPointer = current;
                current = current -> next;
                current->next = last;
                return (void *)returnPointer;
            }
        }
        current = GET_NEXT(current);
    } while (current != search_start);

    return NULL;
}

void simple_free(void *ptr) {
    BlockHeader *block = (BlockHeader*)((uintptr_t)ptr - sizeof(BlockHeader));

    if (GET_FREE(block)) {
        return;
    } else{
        SET_FREE(block, 1);
    }

    BlockHeader *nextBlock = GET_NEXT(block);

    // Forward Coalescing
    if (nextBlock != NULL && GET_FREE(nextBlock)){
        block->next = nextBlock->next;
    }

    // Backward Coalescing
    BlockHeader *previousBlock = first;
    while (previousBlock->next != block){
        previousBlock = GET_NEXT(previousBlock);
    }

    if (previousBlock != NULL && GET_FREE(previousBlock)){
        previousBlock->next = block->next;
    }
}
