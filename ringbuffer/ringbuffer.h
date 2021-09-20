#ifndef HELPERS_RINGBUFFER_H
#define HELPERS_RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RINGBUFF_OK         0
#define RINGBUFF_EMPTY      1
#define RINGBUFF_REFERENCED 2


typedef void (*ring_push_hook_t)(uint8_t *data, size_t size);

typedef struct h_ringbuff_header {
        uint64_t header_size;
        uint64_t ring_size;
        uint64_t internal_ring_size;
        uint64_t w;
        uint64_t r;
        uint64_t size_el;
        bool looping_read;
        bool dynlist_enabled;
        ring_push_hook_t push_hook;
} h_ringbuff_header_t;


/* If the ringbuffer should take elements of a dynamic list,
 * entries also must be lockable. The following structure provides
 * needed entries that have to be prepended to every list element
 * if this functionality is used */
typedef struct h_ringbuff_dynl_hdr {
        struct h_ringbuff_dynl_hdr *next;
        struct h_ringbuff_dynl_hdr *prev;
        uint64_t ref_count;
} h_ringbuff_dynl_hdr_t;


/* DEBUG */
void h_ringbuff_set_push_hook(h_ringbuff_header_t *h, ring_push_hook_t hook);


/*! @brief allocates memory for a buffer
 *  @param size_el     size in bytes of the ringbuffer elements
 *  @param num_el      max number of elements in the buffer
 *  @param header_size size of the ringbuffer header. Just use zero!
 *  @return            pointer to allocated ringbuffer
 */
void *h_ringbuff_alloc(uint64_t size_el, uint64_t num_el, uint64_t header_size);

/*! @brief allocates aligned memory for the ringbuffer
 *  @param alignment   alignment value (must be integer power of 2)
 *  @param size_el     size in bytes of the ringbuffer elements
 *  @param num_el      max number of elements in the buffer
 *  @param header_size size of the ringbuffer header. Use zero to set to size of
 *                     h_ringbuff_header
 *  @return            pointer to allocated ringbuffer
 */
void *h_ringbuff_aligned_alloc(size_t alignment, uint64_t size_el,
                               uint64_t num_el, uint64_t header_size);

/*! @brief initializes a buffer that was previously allocated
 *  @param buffer  pointer to an allocated buffer
 */
void h_ringbuff_init(void *buffer);

/*! @brief enables/disables a ringbuffer's read loop
 *  @param buffer  pointer to allocated ringbuffer
 *  @param loop    desired loop setting
 */
void h_ringbuff_set_read_loop(void *buffer, bool loop);

/*! @brief Frees the ringbuffer that was allocated with \sa *
 *        h_ringbuff_alloc
 *  @param buffer Pointer to the ringbuffer
 */
void h_ringbuff_free(void *buffer);

/*! @brief Frees the ringbuffer that was allocated with \sa
 *  h_ringbuff_aligned_alloc
 *  @param buffer Pointer to the ringbuffer
 */
void h_ringbuff_aligned_free(void *buffer);

/*! @brief Aligns a ringbuffer so that its size is multiples of the element size
 *  @param buffer  Pointer to initialized ringbuffer
 *  @param size_el New size in bytes of the ringbuffer element
 * @return Aligned ringbuffer size
 * @note
 */
uint64_t h_ringbuff_align_size(void *buffer, uint64_t size_el);

uint64_t h_ringbuff_avail(void *buffer);
bool h_ringbuff_is_empty(void *buffer);
bool h_ringbuff_is_full(void *buffer);

uint64_t h_ringbuff_push(void *buffer, void *data, uint64_t size);
void *h_ringbuff_push_ref(void *buffer, void *data);
void *h_ringbuff_read(void *buffer, uint64_t size);
int h_ringbuff_pop(void *buffer, uint64_t size);

void h_ringbuff_size_init(h_ringbuff_header_t *rbh, uint64_t size_el,
                          uint64_t num_el, uint64_t header_size);

size_t h_ringbuff_alloc_size(uint64_t size_el, uint64_t num_el,
                             uint64_t header_size);

void h_ringbuff_set_dynlist_support(void *buffer, bool enable);

void h_ringbuff_dynlist_insert_after(void *buffer, h_ringbuff_dynl_hdr_t *where,
                                     h_ringbuff_dynl_hdr_t *what);

void h_ringbuff_dynlist_insert_before(void *buffer,
                                      h_ringbuff_dynl_hdr_t *where,
                                      h_ringbuff_dynl_hdr_t *what);

void h_ringbuff_dynlist_unlink(void *buffer, h_ringbuff_dynl_hdr_t *what);

#endif
