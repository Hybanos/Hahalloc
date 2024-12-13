#include "hahalloc.h"

void *oversize_alloc(size_t);
void oversize_free(range *);
void  pretty_print(range *r);

// Ranges roots are stored on the stack, which probably is a big limitation
range *roots[RANGE_SEGMENTS];
static size_t oversize_alloc_count;
static bool has_init = false;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int assign_root(size_t req_size) {
    for (int i = 1; i <= RANGE_SEGMENTS; i++) {
        if ((req_size + sizeof(range)) < SMALLEST_SEGMENT << i) {
            return i - 1;
        }
    }
    return RANGE_SEGMENTS - 1;
}

size_t mapping_size_from_index(int index) {
    size_t tmp = SMALLEST_SEGMENT << index;

    if (tmp > PAGESIZE) return tmp * 10;
    return PAGESIZE * 10;
}

void *hahalloc(size_t req_size) {

    // register atexit()
    if (!has_init) {
        atexit(mem_lhihiks);
        has_init = true;
    }

    // Invalid request size
    if (req_size == 0) return NULL;

    // Ranges above 1GB are delivered on their own
    if (req_size > SMALLEST_SEGMENT << RANGE_SEGMENTS) {return oversize_alloc(req_size);}
    pthread_mutex_lock(&lock);

    int index = assign_root(req_size);
    #ifdef DEBUG
    printf("trying to alloc %lu bytes at index %d\n", req_size, index);
    #endif
    range *root;

    // Initialize the first mapping
    if (roots[index] == NULL) {
        void *ptr = mmap(NULL, mapping_size_from_index(index), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memset(ptr, 0, sizeof(range));

        root = ptr;
        root->prev = NULL;
        root->meta = IS_ROOT;
        root->size = mapping_size_from_index(index) - sizeof(range);
        root->next = NULL;

        roots[index] = root;
        
        #ifdef DEBUG
        pretty_print(root);
        #endif
    } else {
        root = roots[index];
    }


    // Linked list parsing to find nearest available range
    range *curr = root;
    while (1) {
        // current range is not allocated, our data might fit
        if (!(curr->meta & IS_ALLOCATED)) {
            // it fits !
            if (curr->size > req_size + sizeof(range)) {

                // The current free range can fit our requested size, but,
                // if the remaining free range after allocation is smaller than
                // the root allows to be allocated, it will never be needed and
                // will slow the linked-list parsing. We then just have to flip
                // this range and return its pointer.
                if ((curr->size + sizeof(range)) - (req_size + sizeof(range)) <
                    SMALLEST_SEGMENT << (index + 1) / 2) {
                    // Flipping current range
                    curr->meta |= IS_ALLOCATED;
                    #ifdef DEBUG
                    pretty_print(root);
                    #endif
                    pthread_mutex_unlock(&lock);
                    return ((void *) curr) + sizeof(range);
                } else {
                    // We have to split the free range into an allocated one and a
                    // free one

                    // new range contains the free area after current range
                    range *new;
                    new = (void *) curr + sizeof(range) + req_size;
                    memset(new, 0, sizeof(range));
                    new->size = curr->size - sizeof(range) - req_size;
                    new->prev = curr;
                    new->next = curr->next;
                    if (curr->next != NULL) {
                        range *next = curr->next;
                        next->prev = new;
                    }

                    // current range now contains the soon-to-be allocated data
                    curr->meta |= IS_ALLOCATED;
                    curr->size = req_size;
                    curr->next = new;

                    // We're good !
                    #ifdef DEBUG
                    pretty_print(root);
                    #endif
                    pthread_mutex_unlock(&lock);
                    return ((void *) curr) + sizeof(range);
                }
            }
        }

        // We went through the wole list and found no space, we need another mapping
        if (curr->next == NULL) {

            #ifdef DEBUG
            printf("Out of memory, getting new mapping...\n");
            #endif

            void *ptr = mmap(NULL, mapping_size_from_index(index), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            range *new = ptr;
            new->prev = curr;
            new->meta = IS_MAPPING_START;
            new->size = mapping_size_from_index(index) - sizeof(range);
            new->next = NULL;

            curr->next = new;
        }
        curr = curr->next;
    }

    #ifdef DEBUG
    pretty_print(root);
    #endif
    pthread_mutex_unlock(&lock);
    return ((void *) curr) + sizeof(range);
}

void frhehe(void *ptr) {
    #ifdef DEBUG
    printf("trying to free %p\n", ptr - sizeof(range));
    #endif

    // Invalid pointer
    if (ptr == NULL) return;
    pthread_mutex_lock(&lock);

    range *curr = ptr - sizeof(range);
    range *prev, *next;
    curr->meta &= ~IS_ALLOCATED;
    prev = curr->prev;
    next = curr->next;

    // Oversize mappings need to have a slightly different treatment
    if (curr->meta & IS_OVERSIZE) {
        oversize_free(curr);
        pthread_mutex_unlock(&lock);
        return;
    }

    // If any neighbooring range is not allocated, we can merge them
    // left neighboor
    if (!(curr->meta & (IS_ROOT | IS_MAPPING_START)) && !(prev->meta & IS_ALLOCATED) ) {
        #ifdef DEBUG
        printf("LEFTIE FREE\n");
        #endif
        prev->size += curr->size + sizeof(range);
        prev->next = next;
        if (next != NULL) {
            next->prev = prev;
        }
        // for futur merging
        curr = prev;
        prev = prev->prev;
    }
    
    // right neighboor
    if (next != NULL && !(next->meta & (IS_ALLOCATED | IS_MAPPING_START))) {
        #ifdef DEBUG
        printf("RIGHTIE FREE\n");
        #endif
        curr->size += next->size + sizeof(range);
        curr->next = next->next;
        next = curr->next;
        if (next != NULL) {
            next->prev = curr;
        }
    }

    // We can munmap the mapping only within a few conditions:
    //  - the range must be at the start of a non-root mapping
    //  - the next range is either another mapping or non-existant
    //       (basically the current range covers the full mapping)
    if (curr->meta & IS_MAPPING_START) {
        #ifdef DEBUG
        printf("Unmmappable range\n");
        #endif
        if (next == NULL) {
            prev->next = NULL;
            munmap(curr, curr->size + sizeof(range));
            curr = prev;
        } else if (next->meta & IS_MAPPING_START) {
            prev->next = next;
            next->prev = prev;
            munmap(curr, curr->size + sizeof(range));
            curr = prev;
        }
    }

    #ifdef DEBUG
    while (curr->prev != NULL) {
        curr = curr->prev;
    }
    pretty_print(curr);
    #endif
    pthread_mutex_unlock(&lock);
}

void *oversize_alloc(size_t req_size) {

    #ifdef DEBUG
    printf("Requesting oversized range HOLD ON\n");
    #endif

    // The size of the range is alligned on the pagesize
    size_t mapping_size = ((req_size + sizeof(range)) / PAGESIZE + 1) * (PAGESIZE);
    range *r = mmap(NULL, mapping_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Setting the metadata to MAPPING_START should get the
    // mapping released when the range is free'd
    r->meta = IS_MAPPING_START | IS_OVERSIZE;
    r->prev = NULL;
    r->next = NULL;
    r->size = mapping_size - sizeof(range);

    oversize_alloc_count++;

    return ((void *) r) + sizeof(range);
}

void oversize_free(range *r) {
    munmap(r, r->size + sizeof(range));
    oversize_alloc_count--;
}

void *rehehalloc(void * ptr, size_t size) {
    #ifdef DEBUG
    printf("trying to realloc %p to size %lu\n", ptr - sizeof(range), size);
    #endif
    range *curr = ptr - sizeof(range);
    range *next = curr->next;

    range back_next = *next;

    // We don't bother updating the pointer if 
    // the requested size is smaller than before.
    if (size <= curr->size) {
        #ifdef DEBUG
        printf("redundent realloc.\n");
        pretty_print(curr);
        #endif
        return ptr;
    }

    // If the following range is free, we can expand into it.
    if (
            !(next->meta & IS_ALLOCATED) && 
            !(next->meta & IS_MAPPING_START) &&
            next->size > size - curr->size) {

        #ifdef DEBUG
        printf("Resize realloc\n");
        #endif

        pthread_mutex_lock(&lock);

        range *new = ptr + size;
        memset(new, 0, sizeof(range));

        new->size = back_next.size - (size - curr->size);
        new->meta = 0;
        new->prev = curr;
        new->next = back_next.next;
        curr->next = new;

        if (back_next.next != NULL) {
            back_next.next->prev = new;
        }
        curr->size = size;
        #ifdef DEBUG
        pretty_print(curr);
        #endif
        pthread_mutex_unlock(&lock);
        return ptr;
    }

    // main case, we allocate a new pointer, memcpy
    // the old data and return it
    void *newptr = hahalloc(size);
    memcpy(newptr, ptr, curr->size);
    frhehe(ptr);
    #ifdef DEBUG
    printf("realloc realloc.\n");
    pretty_print(curr);
    #endif
    return newptr;
}

void *chahalloc(size_t size) {
    void *ptr = hahalloc(size);
    // idk how malloc's memeset is so fast 
    memset(ptr, 0, size);
    return ptr;
}

void mem_lhihiks() {
    size_t active_ranges = oversize_alloc_count;

    printf("atexit HEAP SUMMARY:\n");
    printf(" - %d roots from %ld to %ld Bytes.\n", RANGE_SEGMENTS, SMALLEST_SEGMENT, SMALLEST_SEGMENT << RANGE_SEGMENTS);
    if (!oversize_alloc_count) {
        printf(" - ");    
        GREEN printf("%lu ", oversize_alloc_count); RESET
    }
    else {
        printf(" - ");
        RED printf("%lu ", oversize_alloc_count); RESET
    }
    printf("oversize mapping still active.\n");
    printf(" - roots:\n");


    for (int i = 0; i < RANGE_SEGMENTS; i++) {

        printf("    ROOT [%i]\t", i + 1);
        if (roots[i] == NULL) {
            GRAY printf("(innactive)\n"); RESET
        } else {

            int free_ranges = 0;
            int alloc_ranges = 0;
            int alloc_mappings = 0;
            size_t total_alloc_size = 0;
            size_t total_size = 0;

            range * curr = roots[i];
            do {
                if (curr->meta & IS_MAPPING_START) alloc_mappings++;

                if (curr->meta & IS_ALLOCATED) {
                    alloc_ranges++;
                    total_alloc_size += curr->size;
                    active_ranges++;
                }
                else free_ranges++;

                total_size += curr->size;
                curr = curr->next;

            } while (curr != NULL);

            if (!alloc_ranges) {
                GREEN printf("%d ", alloc_ranges); RESET
            } else {
                RED printf("%d ", alloc_ranges); RESET
            }
            
            printf("allocated range(s) - %d free on %d mappings. (%lu / %lu Bytes).\n", free_ranges, alloc_mappings, total_alloc_size, total_size);
        }
    }

    printf("\n");

    if (active_ranges) {
        printf("There %s %lu active pointer%s in your code right now.\n", (active_ranges == 1 ? "is" : "are"), active_ranges, (active_ranges == 1 ? "" : "s"));
    } else {
        printf("Your heap is completelly clean, good job :D\n");
    }


}

void pretty_print(range *r) {
    range *curr = r;
    size_t total_size = 0;

    if (curr == NULL) {
        printf("NULL\n");
        return;
    }

    while (true) {
        total_size += curr->size + sizeof(range);
        if (curr->meta & IS_MAPPING_START) printf("... -> ");
        if (curr->meta & IS_ROOT) printf("ROOT -> ");
        if (!(curr->meta & IS_ALLOCATED)) GRAY_GRAY printf("[%p - %ld]", curr, curr->size);
        RESET printf(" -> ");

        if (curr->next == NULL) {
            printf("END - %ld\n", total_size);
            return;
        }
        curr = curr->next;
    }
}