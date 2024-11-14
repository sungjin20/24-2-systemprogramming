/*--------------------------------------------------------------------*/
/* chunk.c                                                            */
/* Author: Sungjin Kim                                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "chunk.h"

struct Chunk {
   Chunk_T next;       /* Pointer to the next chunk in the free chunk list */
   int units;          /* Capacity of a chunk (chunk units) */
   int status;         /* CHUNK_FREE or CHUNK_IN_USE */
};

/*--------------------------------------------------------------------*/
int
chunk_get_status(Chunk_T c)
{
   return c->status;
}
/*--------------------------------------------------------------------*/
void
chunk_set_status(Chunk_T c, int status)
{
   c->status = status;
}
/*--------------------------------------------------------------------*/
int
chunk_get_units(Chunk_T c)
{
   return c->units;
}

/*--------------------------------------------------------------------*/
void
chunk_set_units(Chunk_T c, int units)
{
   c->units = units;
}

/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_next_free_chunk(Chunk_T c)
{
  return c->next;
}

/*--------------------------------------------------------------------*/
void
chunk_set_next_free_chunk(Chunk_T c, Chunk_T next)
{
   c->next = next;
}

/*--------------------------------------------------------------------*/
Chunk_T
chunk_get_footer_from_header(Chunk_T c){
   return c + c->units + 1;
}

Chunk_T
chunk_get_header_from_footer(Chunk_T c){
   return c - c->units - 1;
}

Chunk_T
chunk_get_next_adjacent(Chunk_T c, void* start, void* end)
{
   Chunk_T n;

   assert((void *)c >= start);

   /* Note that a chunk consists of two chunks unit for a header and a footer, and
    * many chunk units for data. */
   n = c + c->units + 2;

   /* If 'c' is the last chunk in memory space, then return NULL. */
   if ((void *)n >= end)
      return NULL;
   
   return n;
}

Chunk_T
chunk_get_prev_adjacent(Chunk_T c, void* start, void* end)
{
   Chunk_T n;

   assert((void *)c >= start);

   n = c - 1;

   /* If 'c' is the first chunk in memory space, then return NULL. */
   if ((void *)n < start)
      return NULL;

   /* Note that a chunk consists of two chunks unit for a header and a footer, and
    * many chunk units for data. */
   n = n - n->units - 1;

   return n;
}

#ifndef NDEBUG
/*--------------------------------------------------------------------*/
int 
chunk_is_valid(Chunk_T c, void *start, void *end)
/* Return 1 (TRUE) iff c is valid */
{
   assert(c != NULL);
   assert(start != NULL);
   assert(end != NULL);

   if (c < (Chunk_T)start)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (c >= (Chunk_T)end)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (c->units == 0)
      {fprintf(stderr, "Zero units\n"); return 0; }
   return 1;
}
#endif