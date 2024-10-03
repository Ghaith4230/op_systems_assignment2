#include <stdint.h>
#include "mm.h"

typedef struct header {
    struct header *next;
    uint64_t user_block[0];
} BlockHeader;

#define GET_NEXT(p)    (void *) (p->next & ~0x1)
#define SET_NEXT(p,n)  p->next = (void *) (n & ~0x1)
#define GET_FREE(p)    (uint8_t) ((uintptr_t)(p->next) & 0x1)
#define SET_FREE(p, f) \
    do { \
        if (((uintptr_t)(p->next) & 0x1) != (f)) { \
            p->next = (void *)((uintptr_t)(p->next) ^ 0x1); \
        } \
    } while (0)
#define SIZE(p)        (size_t)(0)
#define MIN_SIZE     (8)

static BlockHeader *first = NULL;
static BlockHeader *current = NULL;

void simple_init() {

    uintptr_t aligned_memory_start = memory_start;
    uintptr_t aligned_memory_end = memory_end;
    BlockHeader *last;

    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            current = (BlockHeader *)(aligned_memory_start + sizeof(BlockHeader));
            current->next = (BlockHeader *)aligned_memory_start;
            first = (BlockHeader *)aligned_memory_start;
            first->next = (BlockHeader *)(aligned_memory_start + sizeof(BlockHeader));
        }
    }
}


void* simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    size_t aligned_size = size;

    BlockHeader *search_start = current;
    do {
        if (GET_FREE(current)) {
            if (SIZE(current) >= aligned_size) {
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    // TODO: Use block as is, marking it non-free
                } else {
                    // TODO: Carve aligned_size from block and allocate new free block for the rest
                }
                return (void *)NULL; // TODO: Return address of current's user_block and advance current
            }
        }
        current = GET_NEXT(current);
    } while (current != search_start);

    return NULL;
}

void simple_free(void *ptr) {
    BlockHeader *block = NULL; // TODO: Find block corresponding to ptr
    if (GET_FREE(block)) {
        return;
    }

    // TODO: Free block

    // Possibly coalesce consecutive free blocks here
}
