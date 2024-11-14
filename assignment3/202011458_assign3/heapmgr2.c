/*--------------------------------------------------------------------*/
/* heapmgr2.c                                                         */
/* Author: Sungjin Kim                                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "chunk.h"

#define FALSE 0
#define TRUE  1

#define BIN_COUNT 10
#define BIN_SIZES {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000}

enum {
   MEMALLOC_MIN = 1024,
};

/* g_bins: array of point to each bin list */
static Chunk_T g_bins[BIN_COUNT] = {NULL};

/* define each bin size */
static const int bin_sizes[BIN_COUNT] = BIN_SIZES;

/* g_free_head: point to first chunk in the free list */
static Chunk_T g_free_head = NULL;

/* g_heap_start, g_heap_end: start and end of the heap area.
 * g_heap_end will move if you increase the heap */
static void *g_heap_start = NULL, *g_heap_end = NULL;

#ifndef NDEBUG
/* check_heap_validity:
 * Validity check for entire data structures for chunks. Note that this
 * is basic sanity check, and passing this test does not guarantee the
 * integrity of your code. 
 * Returns 1 on success or 0 (zero) on failure. 
 */
static int
check_heap_validity(void)
{
   Chunk_T w;

   if (g_heap_start == NULL) {
      fprintf(stderr, "Uninitialized heap start\n");
      return FALSE;
   }

   if (g_heap_end == NULL) {
      fprintf(stderr, "Uninitialized heap end\n");
      return FALSE;
   }

   if (g_heap_start == g_heap_end) {
      if (g_free_head == NULL)
         return 1;
      fprintf(stderr, "Inconsistent empty heap\n");
      return FALSE;
   }

   for (w = (Chunk_T)g_heap_start; 
        w && w < (Chunk_T)g_heap_end;
        w = chunk_get_next_adjacent(w, g_heap_start, g_heap_end)) {

      if (!chunk_is_valid(w, g_heap_start, g_heap_end)) 
         return 0;
   }

   for (w = g_free_head; w; w = chunk_get_next_free_chunk(w)) {
      Chunk_T n;
      if (chunk_get_status(w) != CHUNK_FREE) {
         fprintf(stderr, "Non-free chunk in the free chunk list\n");
         return 0;
      }

      if (!chunk_is_valid(w, g_heap_start, g_heap_end))
         return 0;

      n = chunk_get_next_adjacent(w, g_heap_start, g_heap_end);
      if (n != NULL && n == chunk_get_next_free_chunk(w)) {
         fprintf(stderr, "Uncoalesced chunks\n");
         return 0;
      }
   }

   // check validity of each bin list
   for(int i=0; i<BIN_COUNT; i++){
      for (w = g_bins[i]; w; w = chunk_get_next_free_chunk(w)) {
         Chunk_T n;
         if (chunk_get_status(w) != CHUNK_FREE) {
            fprintf(stderr, "Non-free chunk in the free chunk bin\n");
            return 0;
         }

         if (!chunk_is_valid(w, g_heap_start, g_heap_end))
            return 0;

         n = chunk_get_next_adjacent(w, g_heap_start, g_heap_end);
         if (n != NULL && n == chunk_get_next_free_chunk(w)) {
            fprintf(stderr, "Uncoalesced chunks\n");
            return 0;
         }
      }
   }
   return TRUE;
}
#endif

/* get_bin_index: return the appropriate index of bin of the unit */
static int get_bin_index(int unit) {
    for (int i = 0; i < BIN_COUNT; i++) {
        if (unit <= bin_sizes[i]) {
            return i;
        }
    }
    return -1;  // Return -1 if unit doesn't fit into any bin
}
/*--------------------------------------------------------------*/
/* size_to_units:
 * Returns capable number of units for 'size' bytes. 
 */
/*--------------------------------------------------------------*/
static size_t
size_to_units(size_t size)
{
  return (size + (CHUNK_UNIT-1))/CHUNK_UNIT;
}
/*--------------------------------------------------------------*/
/* get_chunk_from_data_ptr:
 * Returns the header pointer that contains data 'm'. 
 */
/*--------------------------------------------------------------*/
static Chunk_T
get_chunk_from_data_ptr(void *m)
{
  return (Chunk_T)((char *)m - CHUNK_UNIT);
}
/*--------------------------------------------------------------------*/
/* init_my_heap: 
 * Initialize data structures and global variables for
 * chunk management. 
 */
/*--------------------------------------------------------------------*/
static void
init_my_heap(void)
{
   /* Initialize g_heap_start and g_heap_end */
   g_heap_start = g_heap_end = sbrk(0);
   if (g_heap_start == (void *)-1) {
      fprintf(stderr, "sbrk(0) failed\n");
      exit(-1);
   }
}
/*--------------------------------------------------------------------*/
/* merge_chunk:
 * Merge two adjacent chunks and move the merged chunk to appropriate bin if needed.
 * return the merged chunk.
 * Returns NULL on error. 
 */
/*--------------------------------------------------------------------*/
static void
insert_chunk(Chunk_T c); // function prototype

static Chunk_T
merge_chunk(Chunk_T c1, Chunk_T c2)
{
   /* c1 and c2 must be be adjacent */
   assert (c1 < c2 && chunk_get_next_adjacent(c1, g_heap_start, g_heap_end) == c2);
   assert (chunk_get_status(c1) == CHUNK_FREE);
   assert (chunk_get_status(c2) == CHUNK_FREE);

   int c_bin_idx = get_bin_index(chunk_get_units(c1));
   Chunk_T prev = chunk_get_next_free_chunk(chunk_get_footer_from_header(c1));
   Chunk_T next = chunk_get_next_free_chunk(c2);

   /* adjust the units and the next pointer of c1 */
   int new_unit = chunk_get_units(c1) + chunk_get_units(c2) + 2;
   chunk_set_units(c1, new_unit);
   chunk_set_units(chunk_get_footer_from_header(c2), new_unit);

   if(prev == NULL){
      if(c_bin_idx != -1)
         g_bins[c_bin_idx] = next;
      else
         g_free_head = next;
   }
   else{
      chunk_set_next_free_chunk(prev, next);
   }

   if(next != NULL){
      chunk_set_next_free_chunk(chunk_get_footer_from_header(next), prev);
   }

   insert_chunk(c1);
   return c1;
}
/*--------------------------------------------------------------------*/
/* split_chunk:
 * Split 'c' into two chunks s.t. the size of one chunk is 'units' and
 * the size of the other chunk is (original size - 'units' - 1).
 * move each chunk to appropriate bin if needed.
 * returns the chunk with 'units'
 * Returns the data chunk. */
/*--------------------------------------------------------------------*/
static Chunk_T 
split_chunk(Chunk_T c, size_t units)
{
   Chunk_T c2;
   Chunk_T c_footer;
   int all_units;
   int c_bin_idx = get_bin_index(chunk_get_units(c));

   assert (c >= (Chunk_T)g_heap_start && c <= (Chunk_T)g_heap_end);
   assert (chunk_get_status(c) == CHUNK_FREE);
   assert (chunk_get_units(c) > units + 2); /* assume chunk with header unit */
   
   /* adjust the size of the first chunk */
   all_units = chunk_get_units(c);
   int c1_bin_idx = get_bin_index(all_units - units - 2);
   chunk_set_units(c, all_units - units - 2);
   c_footer = chunk_get_footer_from_header(c);
   chunk_set_units(c_footer, all_units - units - 2);

   /* prepare for the second chunk */
   c2 = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
   chunk_set_units(c2, units);
   chunk_set_units(chunk_get_footer_from_header(c2), units);
   chunk_set_status(c2, CHUNK_IN_USE);
   
   Chunk_T c1_next = chunk_get_next_free_chunk(c);
   Chunk_T c1_prev = chunk_get_next_free_chunk(chunk_get_footer_from_header(c2));
   if(c_bin_idx != c1_bin_idx){
      if(c1_prev == NULL){
         if(c_bin_idx != -1)
            g_bins[c_bin_idx] = c1_next;
         else
            g_free_head = c1_next;
      }
      else{
         chunk_set_next_free_chunk(c1_prev, c1_next);
      }
      if(c1_next != NULL)
         chunk_set_next_free_chunk(chunk_get_footer_from_header(c1_next), c1_prev);

      insert_chunk(c);
   }
   else{
      chunk_set_next_free_chunk(c_footer, c1_prev);
   }

   return c2;
}
/*--------------------------------------------------------------------*/
/* split_chunk2:
 * Special split_chunk function that only use in malloc function
 * only c_bin_idx is different from original */
/*--------------------------------------------------------------------*/
static Chunk_T 
split_chunk2(Chunk_T c, size_t units)
{
   Chunk_T c2;
   Chunk_T c_footer;
   int all_units;
   int c_bin_idx = -1;

   assert (c >= (Chunk_T)g_heap_start && c <= (Chunk_T)g_heap_end);
   assert (chunk_get_status(c) == CHUNK_FREE);
   assert (chunk_get_units(c) > units + 2); /* assume chunk with header unit */
   
   /* adjust the size of the first chunk */
   all_units = chunk_get_units(c);
   int c1_bin_idx = get_bin_index(all_units - units - 2);
   chunk_set_units(c, all_units - units - 2);
   c_footer = chunk_get_footer_from_header(c);
   chunk_set_units(c_footer, all_units - units - 2);

   /* prepare for the second chunk */
   c2 = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
   chunk_set_units(c2, units);
   chunk_set_units(chunk_get_footer_from_header(c2), units);
   chunk_set_status(c2, CHUNK_IN_USE);
   
   Chunk_T c1_next = chunk_get_next_free_chunk(c);
   Chunk_T c1_prev = chunk_get_next_free_chunk(chunk_get_footer_from_header(c2));
   if(c_bin_idx != c1_bin_idx){
      if(c1_prev == NULL){
         if(c_bin_idx != -1)
            g_bins[c_bin_idx] = c1_next;
         else
            g_free_head = c1_next;
      }
      else{
         chunk_set_next_free_chunk(c1_prev, c1_next);
      }
      if(c1_next != NULL)
         chunk_set_next_free_chunk(chunk_get_footer_from_header(c1_next), c1_prev);

      insert_chunk(c);
   }
   else{
      chunk_set_next_free_chunk(c_footer, c1_prev);
   }

   return c2;
}
/*--------------------------------------------------------------------*/
/* insert_chunk:
 * Insert a chunk, 'c', into the head of the bin or free chunk list. 
 * 'c' will be merged with the first chunk if possible.
 * The status of 'c' is set to CHUNK_FREE
 */
/*--------------------------------------------------------------------*/
static void
insert_chunk(Chunk_T c)
{
   assert (chunk_get_units(c) >= 1);

   chunk_set_status(c, CHUNK_FREE);
   int c_bin_idx = get_bin_index(chunk_get_units(c));
   Chunk_T c_bin_head;
   if(c_bin_idx != -1){
      c_bin_head = g_bins[c_bin_idx];
   }
   else
      c_bin_head = g_free_head;

   /* If the free chunk list is empty, chunk c points to itself. */
   if (c_bin_head == NULL) {
      if(c_bin_idx != -1){
         g_bins[c_bin_idx] = c;
      }
      else{
         g_free_head = c;
      }
      chunk_set_next_free_chunk(c, NULL);
      chunk_set_next_free_chunk(chunk_get_footer_from_header(c), NULL);
   }
   else {
      chunk_set_next_free_chunk(c, c_bin_head);
      chunk_set_next_free_chunk(chunk_get_footer_from_header(c), NULL);
      chunk_set_next_free_chunk(chunk_get_footer_from_header(c_bin_head), c);

      if(c_bin_idx != -1)
         g_bins[c_bin_idx] = c;
      else
         g_free_head = c;

      if (chunk_get_next_adjacent(c, g_heap_start, g_heap_end) == c_bin_head) 
         merge_chunk(c, c_bin_head);
   }
}
/*--------------------------------------------------------------------*/
/* insert_chunk_after:
 * Insert a chunk, 'c', after 'e' which is
 * already in the bin or free chunk list. After the operation, 'c' will be the
 * next element of 'e' in the bin or free chunk list.  
 * 'c' will be merged with neighbor free chunks if possible.
 * It will return 'c' (after the merge operation if possible).
 */
/*--------------------------------------------------------------------*/
static Chunk_T
insert_chunk_after(Chunk_T e, Chunk_T c)
{
   Chunk_T n;

   assert (e < c);
   assert (chunk_get_status(e) == CHUNK_FREE);
   assert (chunk_get_status(c) != CHUNK_FREE);

   chunk_set_next_free_chunk(c, chunk_get_next_free_chunk(e));
   if(chunk_get_next_free_chunk(e) != NULL){
      chunk_set_next_free_chunk(chunk_get_footer_from_header(chunk_get_next_free_chunk(e)), c);
   }
   chunk_set_next_free_chunk(e, c);
   chunk_set_next_free_chunk(chunk_get_footer_from_header(c), e);
   chunk_set_status(c, CHUNK_FREE);

   /* see if c can be merged with e */
   if (chunk_get_next_adjacent(e, g_heap_start, g_heap_end) == c) 
      c = merge_chunk(e, c);
   
   /* see if we can merge with n */
   n = chunk_get_next_adjacent(c, g_heap_start, g_heap_end);
   if (n != NULL && chunk_get_status(n) == CHUNK_FREE)
      c = merge_chunk(c, n);

   return c;
}
/*--------------------------------------------------------------------*/
/* remove_chunk_from_list:
 * Removes 'c' from the bin or free chunk list. 'prev' should be the previous
 * free chunk of 'c' or NULL if 'c' is the first chunk
 */
/*--------------------------------------------------------------------*/
static void
remove_chunk_from_list(Chunk_T prev, Chunk_T c)
{
   assert (chunk_get_status(c) == CHUNK_FREE);

   int c_bin_idx = get_bin_index(chunk_get_units(c));

   Chunk_T next = chunk_get_next_free_chunk(c);

   if(prev == NULL){
      if(c_bin_idx != -1)
         g_bins[c_bin_idx] = next;
      else
         g_free_head = next;
   }
   else{
      chunk_set_next_free_chunk(prev, next);
   }

   if(next != NULL){
      chunk_set_next_free_chunk(chunk_get_footer_from_header(next), prev);
   }

   chunk_set_next_free_chunk(c, NULL);
   chunk_set_next_free_chunk(chunk_get_footer_from_header(c), NULL);
   chunk_set_status(c, CHUNK_IN_USE);
}
/*--------------------------------------------------------------------*/
/* remove_chunk_from_list2:
 * Special remove_chunk_from_list function that only use in malloc function
 * only c_bin_idx is different from original */
/*--------------------------------------------------------------------*/
static void
remove_chunk_from_list2(Chunk_T prev, Chunk_T c)
{
   assert (chunk_get_status(c) == CHUNK_FREE);

   int c_bin_idx = -1;

   Chunk_T next = chunk_get_next_free_chunk(c);

   if(prev == NULL){
      if(c_bin_idx != -1)
         g_bins[c_bin_idx] = next;
      else
         g_free_head = next;
   }
   else{
      chunk_set_next_free_chunk(prev, next);
   }

   if(next != NULL){
      chunk_set_next_free_chunk(chunk_get_footer_from_header(next), prev);
   }

   chunk_set_next_free_chunk(c, NULL);
   chunk_set_next_free_chunk(chunk_get_footer_from_header(c), NULL);
   chunk_set_status(c, CHUNK_IN_USE);
}
/*--------------------------------------------------------------------*/
/* allocate_more_memory: 
 * Allocate a new chunk which is capable of holding 'units' chunk
 * units in memory by increasing the heap, and return the new
 * chunk. 'prev' should be the last chunk in the free list.
 * This function also performs chunk merging with "prev" if possible
 * after allocating a new chunk. 
*/
/*--------------------------------------------------------------------*/
static Chunk_T
allocate_more_memory(Chunk_T prev, size_t units)
{
   Chunk_T c;
   Chunk_T c_footer;

   if (units < MEMALLOC_MIN)
      units = MEMALLOC_MIN;
   
   /* Note that we need to allocate one more unit for header. */
   c = (Chunk_T)sbrk((units + 1) * CHUNK_UNIT);
   if (c == (Chunk_T)-1)
      return NULL;
   c_footer = (Chunk_T)sbrk(CHUNK_UNIT);
   if (c_footer == (Chunk_T)-1)
      return NULL;
   
   g_heap_end = sbrk(0);
   chunk_set_units(c, units);
   chunk_set_units(c_footer, units);
   chunk_set_next_free_chunk(c, NULL);
   chunk_set_next_free_chunk(c_footer, NULL);
   chunk_set_status(c, CHUNK_IN_USE);
   
   /* Insert the newly allocated chunk 'c' to the free list.
    * Note that the list is sorted in ascending order. */
   
   if (g_free_head == NULL){
      g_free_head = c;
      chunk_set_next_free_chunk(c, NULL);
      chunk_set_next_free_chunk(chunk_get_footer_from_header(c), NULL);
      chunk_set_status(c, CHUNK_FREE);
   }
      
   else 
      c = insert_chunk_after(prev, c);

   assert(check_heap_validity());
   return c;
}
/*--------------------------------------------------------------*/
/* heapmgr_malloc:
 * Dynamically allocate a memory capable of holding size bytes. 
 * Substitute for GNU malloc().                                 
 */
/*--------------------------------------------------------------*/
void *heapmgr_malloc(size_t size) {
    static int is_init = FALSE;
    Chunk_T c;
    size_t units;
    
    if (size <= 0)
        return NULL;

    if (!is_init) {
        init_my_heap();
        is_init = TRUE;
    }

   /* see if everything is OK before doing any operations */
    assert(check_heap_validity());

    units = size_to_units(size);

    /* Check if the request can be satisfied by a bin */
    int bin_index = get_bin_index(units);
    if (bin_index != -1) {
      for(int i=bin_index; i<BIN_COUNT; i++){
         if (g_bins[i] != NULL) {
            Chunk_T prev2 = NULL;
            for (c = g_bins[i]; c != NULL; c = chunk_get_next_free_chunk(c)) {
               if (chunk_get_units(c) >= units) {
                  if (chunk_get_units(c) > units + 2)
                     c = split_chunk(c, units);
                  else
                     remove_chunk_from_list(prev2, c);

                  assert(check_heap_validity());
                  return (void *)((char *)c + CHUNK_UNIT);
               }
               prev2 = c;
            }
         }
      }
      /* Try to allocate from the appropriate bin */
    }

    /* Traverse the free list for large allocations */
    Chunk_T prev = NULL;
    Chunk_T pprev = NULL;
    for (c = g_free_head; c != NULL; c = chunk_get_next_free_chunk(c)) {
        if (chunk_get_units(c) >= units) {
            if (chunk_get_units(c) > units + 2) 
                c = split_chunk(c, units);
            else
                remove_chunk_from_list(prev, c);

            assert(check_heap_validity());
            return (void *)((char *)c + CHUNK_UNIT);
        }
        pprev = prev;
        prev = c;
    }

    /* allocate new memory */
    c = allocate_more_memory(prev, units);
    if (c == NULL) {
        assert(check_heap_validity());
        return NULL;
    }
    assert(chunk_get_units(c) >= units);

   /* if c was merged with prev, pprev becomes prev */
    if (c == prev)
      prev = pprev;

   if (chunk_get_units(c) > units + 2) 
      c = split_chunk2(c, units);
   else 
      remove_chunk_from_list2(prev, c);

    assert(check_heap_validity());
    return (void *)((char *)c + CHUNK_UNIT);
}
/*--------------------------------------------------------------*/
/* heapmgr_free:
 * Releases dynamically allocated memory. 
 * Substitute for GNU free().                                   */
/*--------------------------------------------------------------*/
void heapmgr_free(void *m) {
    Chunk_T c;

    if (m == NULL)
        return;

   /* check everything is OK before freeing 'm' */
    assert(check_heap_validity());

   /* get the chunk header pointer from m */
    c = get_chunk_from_data_ptr(m);
    assert(chunk_get_status(c) != CHUNK_FREE);

    /* Determine if the chunk should be placed in a bin */
   int bin_idx = get_bin_index(chunk_get_units(c));
   if(bin_idx != -1){
      insert_chunk(c);
   }
   else{
      /* traverse the free chunk list to find the
         right location for 'c' */
      Chunk_T prev = c;
      while(chunk_get_status(prev) != CHUNK_FREE){
         prev = chunk_get_prev_adjacent(prev, g_heap_start, g_heap_end);
         if(prev == NULL) break;
      }

      /* insert the new free chunk into the free list */
      if (prev == NULL)
         insert_chunk(c);
      else{
         insert_chunk_after(prev, c);
      }
   }

   /* double check if everything is OK */
    assert(check_heap_validity());
}