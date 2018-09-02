#include "threads/malloc.h"
#include <debug.h>
#include <list.h>
#include <round.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* A simple implementation of malloc().

   The size of each request, in bytes, is rounded up to a power
   of 2 and assigned to the "descriptor" that manages blocks of
   that size.  The descriptor keeps a list of free blocks.  If
   the free list is nonempty, one of its blocks is used to
   satisfy the request.

   Otherwise, a new page of memory, called an "arena", is
   obtained from the page allocator (if none is available,
   malloc() returns a null pointer).  The new arena is divided
   into blocks, all of which are added to the descriptor's free
   list.  Then we return one of the new blocks.

   When we free a block, we add it to its descriptor's free list.
   But if the arena that the block was in now has no in-use
   blocks, we remove all of the arena's blocks from the free list
   and give the arena back to the page allocator.

   We can't handle blocks bigger than 2 kB using this scheme,
   because they're too big to fit in a single page with a
   descriptor.  We handle those by allocating contiguous pages
   with the page allocator and sticking the allocation size at
   the beginning of the allocated block's arena header. */

/* Descriptor. */
struct desc
  {
    size_t block_size;          /* Size of each element in bytes. */
    size_t blocks_per_arena;    /* Number of blocks in an arena. */
    struct list free_list;      /* List of free blocks. */
    struct lock lock;           /* Lock. */
  };

/* Magic number for detecting arena corruption. */
#define ARENA_MAGIC 0x9a548eed

/* Arena. */
struct arena 
  {
    unsigned magic;             /* Always set to ARENA_MAGIC. */
    struct desc *desc;          /* Owning descriptor, null for big block. */
    size_t free_cnt;            /* Free blocks; pages in big block. */
  };

/* Free block. */
struct block 
  {
    size_t size;
    struct list_elem free_elem; /* Free list element. */
    unsigned magic;

  };

/* Our set of descriptors. */
static struct desc descs[10];   /* Descriptors. */
static size_t desc_cnt;         /* Number of descriptors. */
static size_t total_pages;
static struct lock page_lock;
static struct arena *block_to_arena (struct block *);
static struct block *arena_to_block (struct arena *, size_t idx);

void list_order_insert(struct list *l,struct list_elem *e)
{
  struct list_elem *temp=list_begin(l);
  while(temp!=list_end(l))
  {
    if(e<temp)
      return list_insert(temp,e);
    temp=list_next(temp);
  }
  return list_insert(temp,e);

}


void *getPage(struct list_elem * e[])
{
  struct list_elem *temp=e[0];
  int i=0,j=0;
  while(i<desc_cnt)
  {
    if(e[i]!=list_end(&descs[i].free_list))//last element in free list
    {
      temp=j==0?e[i]:temp;
      if(j==0)
      {
        ++j;
        continue;
      }

      temp=e[i]<temp ?e[i]:temp;

    }
    ++i;
  }
  return pg_round_down(temp);
}
void printMemory(void)
{
  

  struct list_elem* e[10];
  int i=0;
  while(i<desc_cnt)
  {
    e[i]=list_begin(&descs[i].free_list);
    ++i;
  }
  printf("total pages allocated:%d\n",total_pages );
  for(i=total_pages-1;i>=0;--i)
  {
    printf("Page number=%d\n",total_pages-i);
    int j=0;
    while(j<desc_cnt)
    {
       printf("Size %d: ", descs[j].block_size);
      
      for(;e[j] != list_end(&descs[j].free_list) && getPage(e) == pg_round_down(e[j]);e[j] = list_next(e[j]))
        printf("%p", e[j]);
        

      ++j;
      printf("\n");
    }
    printf("\n");
  }

}
/* Initializes the malloc() descriptors. */
void
malloc_init (void) 
{
  size_t block_size;
  total_pages=0;
  lock_init (&page_lock);
  for (block_size = 16; block_size <= PGSIZE / 2; block_size *= 2)
    {
      struct desc *d = &descs[desc_cnt++];
      ASSERT (desc_cnt <= sizeof descs / sizeof *descs);
      d->block_size = block_size;
      d->blocks_per_arena = (PGSIZE - sizeof (struct arena)) / block_size;
      list_init (&d->free_list);
      lock_init (&d->lock);
    }

}

void 
malloc_helper(struct desc *d,struct block *b,size_t required,size_t split)
{
  lock_acquire (&(d-1)->lock);
  struct block* free_block=(struct block *) ((uint8_t *)b+ split/2);
  free_block->size=split/2;
  printf("%d BLOCK SIZE\n",free_block->size );
  list_order_insert(&(d-1)->free_list,&free_block->free_elem);
  lock_release(&(d-1)->lock);
  if(required!=split/2)
  {

    malloc_helper(d-1,b,required,split/2);
  }
  return;

}


/* Obtains and returns a new block of at least SIZE bytes.
   Returns a null pointer if memory is not available. */
void *
malloc(size_t size) 
{
  struct desc *d;
  struct block *b;
  //struct arena *a;

  /* A null pointer satisfies a request for 0 bytes. */
  if (size == 0)
    return NULL;

  /* Find the smallest descriptor that satisfies a SIZE-byte
     request. */
  size=size+sizeof(struct block);
  for (d = descs; d < descs + desc_cnt; d++)
    if (d->block_size >= size)
      break;
  printf("\nRequired block size=%d\n",d->block_size);

  if (d == descs + desc_cnt) 
    {
      /* SIZE is too big for any descriptor.
         Allocate enough pages to hold SIZE plus an arena. */
      size_t page_count = DIV_ROUND_UP (size + sizeof *b, PGSIZE);
      printf("Multi page allocation no of pages allocated=%d\n",page_count );
      b = palloc_get_multiple (0, page_count);
      if (b == NULL)
        return NULL;
      b->size=PGSIZE*page_count;
      b->magic=ARENA_MAGIC;
      /* Initialize the arena to indicate a big block of PAGE_CNT
         pages, and return it. */
      
  printf("%d size allocated\n", b->size);
      return b+1;//check
    }

  lock_acquire (&d->lock);

  struct desc *temp;
  for(temp=d;temp!=descs+desc_cnt;)
  {
     lock_release (&temp->lock);
    if(!list_empty(&temp->free_list))// free block of required size found
      {
        printf("Existing freelist found block size=%d\n",temp->block_size);
        b = list_entry (list_pop_front (&temp->free_list), struct block, free_elem);
        if(temp!=d)
        {
          malloc_helper(temp,b,d->block_size,b->size);
        }
        
        b->magic=ARENA_MAGIC;
        b->size=d->block_size;
        printf("%d size allocated\n", b->size);
        return b+1;
      }
      temp++;
      if(temp!=descs+desc_cnt)
      lock_acquire (&temp->lock);
  }
  printf("Allocating in new page. No free list of reqd size exists\n");
  b = palloc_get_page (0);
  if (b == NULL) 
  {
    lock_release (&temp->lock);
    return NULL; 
  }
  lock_acquire(&page_lock);
  total_pages++;
  lock_release(&page_lock);


  malloc_helper(temp,b,d->block_size,PGSIZE);
 
  if(temp!=descs+desc_cnt)
      lock_release(&temp->lock);
  b->size = d->block_size;
  b->magic=ARENA_MAGIC;
  printf("%d size allocated\n", b->size);
  return b+1;

  /* If the free list is empty, create a new arena. */
//   if (list_empty (&d->free_list))
//     {
//       size_t i;

//       /* Allocate a page. */
//       a = palloc_get_page (0);
//       if (a == NULL) 
//         {
//          
//           return NULL; 
//         }

//       /* Initialize arena and add its blocks to the free list. */
//       a->magic = ARENA_MAGIC;
//       a->desc = d;
//       a->free_cnt = d->blocks_per_arena;
//       for (i = 0; i < d->blocks_per_arena; i++) 
//         {
//           struct block *b = arena_to_block (a, i);
//           list_push_back (&d->free_list, &b->free_elem);
//         }
      
      
//     }

//   /* Get a block from free list and return it. */
//   b = list_entry (list_pop_front (&d->free_list), struct block, free_elem);
//   a = block_to_arena (b);
//   a->free_cnt--;
//   return b;
 }


/* Allocates and return A times B bytes initialized to zeroes.
   Returns a null pointer if memory is not available. */
void *
calloc (size_t a, size_t b) 
{
  void *p;
  size_t size;

  /* Calculate block size and make sure it fits in size_t. */
  size = a * b;
  if (size < a || size < b)
    return NULL;

  /* Allocate and zero memory. */
  p = malloc (size);
  if (p != NULL)
    memset (p, 0, size);

  return p;
}

/* Returns the number of bytes allocated for BLOCK. */
static size_t
block_size (void *block) 
{
  struct block *b = block;
  struct arena *a = block_to_arena (b);
  struct desc *d = a->desc;

  return d != NULL ? d->block_size : PGSIZE * a->free_cnt - pg_ofs (block);
}

/* Attempts to resize OLD_BLOCK to NEW_SIZE bytes, possibly
   moving it in the process.
   If successful, returns the new block; on failure, returns a
   null pointer.
   A call with null OLD_BLOCK is equivalent to malloc(NEW_SIZE).
   A call with zero NEW_SIZE is equivalent to free(OLD_BLOCK). */
void *
realloc (void *old_block, size_t new_size) 
{
  if (new_size == 0) 
    {
      free (old_block);
      return NULL;
    }
  else 
    {
      void *new_block = malloc (new_size);
      if (old_block != NULL && new_block != NULL)
        {
          size_t old_size =  ((struct block*)(old_block)-1)->size;
          size_t min_size = new_size < old_size ? new_size : old_size;
          memcpy (new_block, old_block, min_size);
          free (old_block);
        }
      return new_block;
    }
}



void 
free_helper(struct block* b, struct desc* d)
{
  ASSERT(b->magic==ARENA_MAGIC);
  printf("merge blocks size=%d\n",b->size);
  struct list_elem *e;
  e=list_begin(&d->free_list);
  struct block* buddy=NULL;
  while(e!=list_end(&d->free_list))
  {
    buddy=list_entry(e,struct block, free_elem);
    size_t s1=(size_t)((uintptr_t)buddy ^ (uintptr_t)b);
    //printf("%d %d\n",s1, b->size );
    if(b->size==s1)
    {
      //printf("hello\n");
      printf("Buddy found size=%d .merging ....\n\n",b->size );
      if(buddy)
      list_remove(&buddy->free_elem);
      if(buddy<b)
        b=buddy;
      if(b->size>=PGSIZE/2)
      {
        lock_acquire(&page_lock);
        total_pages--;
        lock_release(&page_lock);
       palloc_free_page(b);
       printf("Page freed\n");
      }
      else
      {
        b->size*=2;

        free_helper(b,d+1);
      }
      return;

    }
    e=list_next(e);
  }

  list_order_insert (&d->free_list, &b->free_elem);
}

/* Frees block P, which must have been previously allocated with
   malloc(), calloc(), or realloc(). */
void
free (void *p) 
{

  //printf("in free\n");
  //printf("hello %d\n",(void *)(p-sizeof(struct block)) );
  if (p != NULL)
    {
      struct block *b = p;
      b=b-1;
      size_t block_size=b->size;

      if(block_size>=PGSIZE)
      {
        //large block spanning multiple pages
        palloc_free_multiple (b, block_size/PGSIZE);
        

      }
      else
      {
        struct desc *d;
        for(d = descs; d < descs + desc_cnt; d++)
        {
          //printf("block size=%d\n",block_size );
          if(d->block_size == block_size)
             {
                lock_acquire (&d->lock);
                
                free_helper(b,d);
                
                lock_release (&d->lock);
             }
        }
        

      }
      
 
    }
}

/* Returns the arena that block B is inside. */
static struct arena *
block_to_arena (struct block *b)
{
  struct arena *a = pg_round_down (b);

  /* Check that the arena is valid. */
  ASSERT (a != NULL);
  ASSERT (a->magic == ARENA_MAGIC);

  /* Check that the block is properly aligned for the arena. */
  ASSERT (a->desc == NULL
          || (pg_ofs (b) - sizeof *a) % a->desc->block_size == 0);
  ASSERT (a->desc != NULL || pg_ofs (b) == sizeof *a);

  return a;
}

/* Returns the (IDX - 1)'th block within arena A. */
static struct block *
arena_to_block (struct arena *a, size_t idx) 
{
  ASSERT (a != NULL);
  ASSERT (a->magic == ARENA_MAGIC);
  ASSERT (idx < a->desc->blocks_per_arena);
  return (struct block *) ((uint8_t *) a
                           + sizeof *a
                           + idx * a->desc->block_size);
}
