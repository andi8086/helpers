# Modular Heap Manager

This heap manager provides a fast way to manage memory blocks (to allocate and
free them) within a given memory space. Due to its context, multiple heaps can
be used in parallel.

Free and allocated blocks are registered in red black binary trees. These are
self-balancing trees, that guarantee search in O(log n) time. Insert and delete
are also performed in O(log n) time.

Free blocks (`fblocks`) are stored with their size as key, allocated blocks
(`ablocks`) are stored with their address as key. Since keys must be unique,
only one fblock is stored in the tree, if there are more free blocks with the
same size. All others are linked to the in-tree-block via a doubly linked
dynamic list.

On allocation, an additional space is allocated preceding the block given to
the user. This is called `prefix`. The tree nodes as well as all other
information is stored inside this `prefix`. Allocated blocks always store
information about the immediately following free block. For this system to
work, we need one allocated block with the size of the prefix in the very
beginning. This is called the `root block`.

No information whatsoever is stored in the free blocks belonging to the heap
manager's context. This memory areas may be freely (ab)used. However, their
beginning is garbled on allocation, because of the prefix.

The context, which makes the heap modular and allows the user to use multiple
heaps in parallel stores the binary trees' roots as well as the base and size
of the managed memory blobb.

** WARNING ** The heap manager is not thread-safe, use your own locking
mechanism around `hm_init`, `hm_alloc` and `hm_free`!


## API

`void hm_init(hm_ctx_t *ctx, void *base, size_t size);`

Initialize heap manager context with given memory base and size. (The 32-bit
version has return type of int and returns -1 in case of error (= if size is
larger than 2G). It returns 0 on success.

`void *hm_alloc(hm_ctx_t *ctx, size_t size);`

Allocate memory area in heap context. size must not be zero.
`NULL` is returned if no space is available of if size was zero.

`void *hm_aligned_alloc(hm_ctx_t *ctx, size_t size, size_t align);`

Allocates memory with `align` bytes alignment. Alignment can be 4G bytes
maximum for 64-bit version and 2G bytes maximum for 32-bit version. Values that
are not a power of 2 lead to undefined behaviour. The alighment of the 32-bit
version only takes place INSIDE the specified memory area, i.e. the area itself
must already be aligned. In the 64-bit version, the alignment is total.

`void hm_free(hm_ctx_t *ctx, void *p);`

Frees alocated memory in heap context.

`uint64_t hm_max(hm_ctx_t *ctx);`

Returns the maximally allocatable memory size.

`uint64_t hm_available(hm_ctx_t *ctx, bool net);`

Returns the total amount of available memory. If `net` is `true`, the amount of
memory needed for heap management is subtracted.

`uint64_t hm_allocated(hm_ctx_t *ctx);`

Returns the total amount of memory allocated, including heap management storage.


## Special debugging features

`HEAPM_MALLOC_LINE_STORE`

If you define this compiler symbol, then `hm_malloc` and `hm_aligned_malloc`
are both macros, that help storing the line number in code into the allocation
information. This way, if memory leaks exist, it is immediately seen, where the
memory was allocated.

`HEAPM_DEBUG`

Define this compiler symbol to make `show_mem` function available. Which prints
detailed current memory stats to console.


: Andreas
