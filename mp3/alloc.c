 /*
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define META_SIZE sizeof(meta_block) // size defined, don't have to keep typing it out
#define align4(x) (((((x)-1)>>2)<<2)+4) // aligns blocks for future purposes

typedef struct meta_block {
    void *prev;
    void *next;
    size_t size;
    int free;
} meta_block;

void *base  = NULL;
void *free_head = NULL;

/*
  helper functions
*/


// creates new block
void *make_space(size_t size) {
    void *block = sbrk(size + META_SIZE);
    if (block == ((void *) -1)) return NULL;

    meta_block *meta  = (meta_block *) block;
    meta->prev = base;
    meta->next = NULL;
    meta->size = size;
    meta->free= 0;

    if (base) ((meta_block *) base)->next = block;
    base = block;
    if (free_head - block > 0) free_head = block; // new block is before current free head, updates free head
    return block + META_SIZE;
}


// splits block
void split_block(size_t size, void *block) {
    meta_block *meta = (meta_block *) block;
    meta_block *new = (meta_block *) (block + META_SIZE + size);

    new->next = (meta_block *) meta->next;
    if ((meta_block *) meta->next) ((meta_block *) meta->next)->prev = new;
    meta->next = new;
    new->prev = meta;
    new->size = meta->size - META_SIZE - size;
    meta->size = size;
    new->free = 1;
}

// reuses free block
void *reuse_space(size_t size, void *block) {
    meta_block *meta = (meta_block *) block;
    if (meta->size >= size + META_SIZE) split_block(align4(size), block);
    meta->free = 0;
    if (free_head - block > 0) free_head = block; // new block is before current free head, updates free head
    return block + META_SIZE;
}
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    size_t m_size = num * size;
    void *ptr = malloc(m_size);
    if (ptr) memset(ptr, 0, m_size);
    return ptr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    if (!size) return NULL;
    size_t s = align4(size);
    void *curr = free_head;
    meta_block *meta;
    while (curr != NULL) {
        meta = (meta_block *) curr;
        if (meta->size >= s && meta->free) return reuse_space(s, meta);
        curr = meta->next;
    }
    return make_space(s);
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (ptr == NULL) return;
    
    void *block = ptr - META_SIZE;
    meta_block *meta = (meta_block *) block;
    if (!free_head) free_head = block;
    else if (free_head -block > 0) free_head = block;
    // merge neighbors if free
    meta_block *next_meta = (meta_block *) meta->next;
    meta_block *prev_meta = (meta_block *) meta->prev;

    if (next_meta && next_meta->free && prev_meta && prev_meta->free) {
        if ((meta_block *) next_meta->next) ((meta_block *) next_meta->next)->prev = prev_meta;
        prev_meta->next = (meta_block *) next_meta->next;
        if (next_meta == base) base = prev_meta;
        prev_meta->size += meta->size + next_meta->size + 2 * META_SIZE;
        return;
    } else if (next_meta && (next_meta != base) && next_meta->free) {
        if ((meta_block *) next_meta->next) ((meta_block *) next_meta->next)->prev = meta;
        meta->next = (meta_block *) next_meta->next;
        meta->size += META_SIZE + next_meta->size;
        meta->free = 1;
        return;
    } else if (prev_meta && prev_meta->free) {
        if (next_meta) next_meta->prev = prev_meta;
        prev_meta->next = next_meta;
        if (meta == base) base = prev_meta;
        prev_meta->size += meta->size + META_SIZE;
        return;
    }
    meta->free = 1;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) return malloc(size);
    if (size == 0) {
      free(ptr);
      return NULL;
    }

    void *block = ptr - META_SIZE;
    meta_block *meta = (meta_block *) block;
    size_t s = align4(size);
    if (meta->size >= s) return ptr;
    void *mem = malloc(size);
    if (mem == NULL) return NULL;
    memcpy(mem, ptr, meta->size);
    free(ptr);

    return mem;
}
