//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head;
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap[4];

//
// Helper functions (feel free to add/remove/edit!)
//

//free binのインデックスを計算する
int find_index_of_the_bin(size_t size) {
    int index = 0;
    if (size < 1000) {
        index = 0;
    }
    else if (size < 2000) {
        index = 1;
    }
    else if (size < 3000) {
        index = 2;
    }
    else {
        index = 3;
    }
    return index;
}

void my_add_to_free_list(my_metadata_t *metadata) {
  int index = find_index_of_the_bin(metadata->size);
  assert(!metadata->next);
  metadata->next = my_heap[index].free_head;
  my_heap[index].free_head = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev) {
  int index = find_index_of_the_bin(metadata->size);
  if (prev) {
    prev->next = metadata->next;
  } else {
    my_heap[index].free_head = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  for (int i = 0; i < 4; i++) {
      my_heap[i].dummy.size = 0;
      my_heap[i].dummy.next = NULL;
      my_heap[i].free_head = &my_heap[i].dummy;
  }
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  int index = find_index_of_the_bin(size);
  my_metadata_t* metadata = my_heap[index].free_head;
  my_metadata_t *prev = NULL;
  // First-fit: Find the first free slot the object fits.
  // TODO: Update this logic to Best-fit! -> This is the implemented version
  size_t best_difference = -1;
  my_metadata_t *best_metadata = NULL;
  my_metadata_t *best_prev = NULL;
  //そのindexを探して見つからない→もう1サイズ大きいbinを探したら見つかるかも
  while (index < 4) {
      metadata = my_heap[index].free_head;
      while (metadata) {
          size_t difference = metadata->size - size;
          if (difference >= 0) {
              if (best_difference > difference || best_difference == -1) {
                  best_difference = difference;
                  best_metadata = metadata;
                  best_prev = prev;
              }
          }
          prev = metadata;
          metadata = metadata->next;
      }
      if (best_difference != -1) {
          break;
      } else {
          index += 1;
      }
  }
  metadata = best_metadata;
  prev = best_prev;
  // now, metadata points to the best slot
  // and prev is the previous entry.

  if (!metadata) {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    // Add the memory region to the free list.
    my_add_to_free_list(metadata);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata + 1;
  size_t remaining_size = metadata->size - size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata, prev);

  if (remaining_size > sizeof(my_metadata_t)) {
    // Shrink the metadata for the allocated object
    // to separate the rest of the region corresponding to remaining_size.
    // If the remaining_size is not large enough to make a new metadata,
    // this code path will not be taken and the region will be managed
    // as a part of the allocated object.
    metadata->size = size;
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    my_add_to_free_list(new_metadata);
  }
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
  /*my_malloc(128);
  my_malloc(128);
  my_malloc(128);
  my_malloc(128);
  my_malloc(128);
  my_malloc(128);
  my_malloc(128);
  my_malloc(4096);
  my_malloc(1024);
  my_malloc(2048);
  printf("Succeed\n")*/
}
