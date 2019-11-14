/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define HDR_SIZE (ALIGN(sizeof(blkHdr)))

#define PSIZE(x) (x&~1)

#define LST_SIZE 5
#define BLK_SIZE (ALIGN(HDR_SIZE+sizeof(size_t)))

typedef struct header blkHdr;

struct header{
   size_t size;
   blkHdr *next;
   blkHdr *prior;
};

blkHdr *find_fit(size_t size);
blkHdr *coalesce(size_t *fblk);
size_t *LRB=0;   

size_t blk_sizes[]={
    BLK_SIZE,
    (8+BLK_SIZE),
    (32+BLK_SIZE),
    (128+BLK_SIZE),
    (256+BLK_SIZE)};
blkHdr *freeLst;

int mm_init(void)
{
  int i;
  freeLst=mem_sbrk(LST_SIZE*HDR_SIZE);
  for(i=0;i<LST_SIZE;i++)
   {
     freeLst[i].size=HDR_SIZE;
     freeLst[i].next=freeLst[i].prior=&freeLst[i];
   }
  return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.LIGNMENT 8
 *
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  int twoP=2;
  int nsize=0;
  while(nsize<=size)
   {
     if(nsize==size)
       {
         nsize=size;
         break;
       }
     nsize=(1<<twoP);
     twoP=twoP+1;
   }

  if(abs(nsize-size)>200)
   {
      nsize=size;
   }

  int newsize = ALIGN(nsize + HDR_SIZE+sizeof(size_t));
  blkHdr *bp = find_fit(newsize);
  if(bp==NULL)
    {
       bp=mem_sbrk(newsize);

       if((long)bp==-1)
         {
           return NULL;
         }
       bp->size=newsize|1;
       size_t *blkFtr=(size_t *)(((char *)bp+PSIZE(bp->size))-ALIGN(sizeof(size_t)));  //getting to blkFtr
       *blkFtr=newsize|1;
       return (void *)((char *)bp+ALIGN(sizeof(size_t)));
    }
 else {

    size_t minblk=PSIZE(bp->size)-newsize;
    if(minblk>HDR_SIZE+ALIGN(sizeof(size_t)))
     {
       bp->size=newsize|1;
       size_t *nblkFtr=(size_t *)(((char *)bp+PSIZE(bp->size))-ALIGN(sizeof(size_t)));  //getting to blkFtr
       *nblkFtr=bp->size;          

       blkHdr *newsplit=(blkHdr *)((char *)bp+PSIZE(bp->size));
       newsplit->size=minblk&~1;

       size_t *blkFtr=(size_t *)(((char *)newsplit+PSIZE(newsplit->size))-ALIGN(sizeof(size_t)));
       *blkFtr=minblk&~1;

       blkHdr *temp=coalesce((size_t *)newsplit);
       if(temp!=NULL)
         {
            newsplit=temp;
         }

       int i;

       for(i=LST_SIZE-1;i>=0;i--)
         {
           if(blk_sizes[i]<=(newsplit->size))
             {
               newsplit->next=freeLst[i].next;
               newsplit->prior=&freeLst[i];
               freeLst[i].next=newsplit;
               (newsplit->next)->prior=newsplit;
               break;
             }
         }
     }
    else      
     {
       bp->size = bp->size | 1;
       size_t *blkFtr=(size_t *)(((char *)bp+PSIZE(bp->size))-ALIGN(sizeof(size_t)));  //getting to blkFtr
       *blkFtr=bp->size|1;
     }
    return (void *)((char *)bp + ALIGN(sizeof(size_t)));
  }
}

blkHdr *find_fit(size_t size)
 {

  int i;
  blkHdr *fp;
  for(i=0;i<LST_SIZE;i++)
   {
     if((blk_sizes[i]>=size && freeLst[i].next!=&freeLst[i]))
      {

         fp=freeLst[i].next;
         freeLst[i].next=fp->next;
         (fp->next)->prior=&freeLst[i];
         return fp;

      }
   }
  fp=freeLst[4].next;
  while(fp!=&freeLst[4])
   {
      if(fp->size>=size)
       {
          (fp->prior)->next=fp->next;
          (fp->next)->prior=fp->prior;
          return fp;
       }
      fp=fp->next;
   }

   return NULL;
 }
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

  int i;
  blkHdr *coalBlk=coalesce((size_t *)((char *)ptr-ALIGN(sizeof(size_t))));
  blkHdr *bp;
  if(coalBlk!=NULL)
    {
      bp=coalBlk;
    }
  else
    {
      bp=(blkHdr *)((char *)ptr-ALIGN(sizeof(size_t)));
    }

  bp->size=(bp->size)&~1;

  for(i=LST_SIZE-1;i>=0;i--)
   {
      if(blk_sizes[i]<=(bp->size))
       {
         size_t *blkFtr=(size_t *)(((char *)bp+PSIZE(bp->size))-ALIGN(sizeof(size_t)));  //getting to blkFtr
         *blkFtr=(bp->size)&~1;

         bp->next=freeLst[i].next;
         bp->prior=&freeLst[i];
         freeLst[i].next=bp;
        (bp->next)->prior=bp;
         return;
       }
   }
}



blkHdr *coalesce(size_t *fblk)  

  {
     size_t *next=(size_t *)((char *)fblk+PSIZE(*fblk));  
     size_t *previous=(size_t *)((char *)fblk-ALIGN(sizeof(size_t)));
     if((void *)((char *)next-1)==mem_heap_hi())     
       {
          if(((char *)fblk-(LST_SIZE*HDR_SIZE))==mem_heap_lo())   
            {
              return NULL;
            }

          else if((*previous&1)==0)
            {
              blkHdr *prevblk=(blkHdr *)((char *)fblk-*previous);

             ((prevblk->prior)->next)=prevblk->next;   
             (prevblk->next)->prior=prevblk->prior;

              prevblk->size=prevblk->size+PSIZE(*fblk);

              size_t *blkFtr=(size_t *)(((char *)prevblk+PSIZE(prevblk->size))-ALIGN(sizeof(size_t)));
              *blkFtr=prevblk->size&~1;   
              return prevblk;

            }
          else
            {
             return NULL;
            }

       }

     else  if(((char *)fblk-(LST_SIZE*HDR_SIZE))==mem_heap_lo())  
            {
               if((*next&1)==0)
                 {
                    blkHdr *nextblk=(blkHdr *)((char *)fblk+PSIZE(*fblk));

                    ((nextblk->prior)->next)=nextblk->next;   
                    (nextblk->next)->prior=nextblk->prior;

                    blkHdr *tempblk=(blkHdr *)fblk;       
                    tempblk->size=nextblk->size+PSIZE(*fblk);

                    size_t *blkFtr=(size_t *)(((char *)tempblk+PSIZE(tempblk->size))-ALIGN(sizeof(size_t)));
                    *blkFtr=tempblk->size&~1;    
                    return tempblk;
                 }
              return NULL;
            }
     else   
       {

           if((*next&1)==0 && (*previous&1)==0)  
            {
               blkHdr *prevblk=(blkHdr *)((char *)fblk-*previous);
               blkHdr *nextblk=(blkHdr *)((char *)fblk+PSIZE(*fblk));

               ((prevblk->prior)->next)=prevblk->next;   
               (prevblk->next)->prior=prevblk->prior;

               ((nextblk->prior)->next)=nextblk->next;   
               (nextblk->next)->prior=nextblk->prior;

               prevblk->size=prevblk->size+PSIZE(*fblk)+nextblk->size;

               size_t *blkFtr=(size_t *)(((char *)prevblk+prevblk->size)-ALIGN(sizeof(size_t)));
               *blkFtr=prevblk->size&~1;               
               return prevblk;

            }
           else if((*next&1)==1 && (*previous&1)==0)
            {

               blkHdr *prevblk=(blkHdr *)((char *)fblk-*previous);

               ((prevblk->prior)->next)=prevblk->next;  
               (prevblk->next)->prior=prevblk->prior;

                prevblk->size=prevblk->size+PSIZE(*fblk);

                size_t *blkFtr=(size_t *)(((char *)prevblk+prevblk->size)-ALIGN(sizeof(size_t)));
                *blkFtr=prevblk->size&~1;    
                return prevblk;

            }
           else if((*next&1)==0 && (*previous&1)==1)
            {
                blkHdr *nextblk=(blkHdr *)((char *)fblk+PSIZE(*fblk));

                ((nextblk->prior)->next)=nextblk->next;  
                (nextblk->next)->prior=nextblk->prior;

                blkHdr *tempblk=(blkHdr *)fblk;    
                tempblk->size=nextblk->size+PSIZE(*fblk);

                size_t *blkFtr=(size_t *)(((char *)tempblk+tempblk->size)-ALIGN(sizeof(size_t)));
                *blkFtr=tempblk->size&~1;   
                return tempblk;
            }
           else
            {
              return NULL;
            }

       }

  }


void print_free()
{
int i;

  for(i=0;i<LST_SIZE;i++)
   {
      printf("For free list of size: %zu \n",blk_sizes[i]);
      blkHdr *bp=freeLst[i].next;
      while(bp!=&freeLst[i])
        {
          printf("%s blk at %p, size %d\n",(bp->size&1)?"allocated":"free",bp,PSIZE((int)bp->size));
          bp=bp->next;
        }
    }
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  if(ptr==NULL)
    {
      return mm_malloc(size);
    }
   else if(size==0)
    {
      mm_free(ptr);
      return NULL;
    }
   else
     {
        size_t *currblk=(size_t *)((char *)ptr-ALIGN(sizeof(size_t)));
        int newsize = ALIGN(size + HDR_SIZE+sizeof(size_t));

        if(PSIZE(*currblk)>=newsize)
         {
            return ptr;
         }
        else if((void *)((char *)currblk+PSIZE(*currblk))==(mem_heap_hi()+1))
         {
           newsize=ALIGN(PSIZE(*currblk)+size);
           mem_sbrk(ALIGN(size));
           *currblk=newsize|1;
           size_t *blkFtr=(size_t *)(((char *)currblk+PSIZE(*currblk))-ALIGN(sizeof(size_t)));  //getting to blkFtr
           *blkFtr=newsize|1;
           return ptr;
         }
        else
         {
             if(LRB==currblk)  //give bigger size than usual
              {

                 newsize=newsize*2; //giving six times more space than requested by user
                 size_t oldsize=PSIZE(*currblk);
                 currblk=mm_malloc(newsize);  //mallocing new blk
                 memcpy(currblk,ptr,oldsize-2*(ALIGN(sizeof(size_t)))); //worried about padding issues
                 mm_free(ptr);
                 size_t *temp=(size_t *)((char *)currblk-ALIGN(sizeof(size_t)));
                 LRB=temp;
                 return currblk;
              }
             else                        //treat as a normal realloc request
              {

                 size_t oldsize=PSIZE(*currblk);
                 currblk=mm_malloc(newsize);  //mallocing new blk
                 memcpy(currblk,ptr,oldsize-2*(ALIGN(sizeof(size_t)))); //worried about padding issues
                 mm_free(ptr);
                 size_t *temp=(size_t *)((char *)currblk-ALIGN(sizeof(size_t)));
                 LRB=temp;
                 return currblk;
              }
         }

     }
}
void print_heap(void *blk)
  {
    if((size_t)blk==-1)
      {
        blkHdr *bp=mem_heap_lo();
        while(bp<(blkHdr *)mem_heap_hi())
          {
            printf("%s blk at %p, size %d\n",(bp->size&1)?"allocated":"free",bp,PSIZE((int)bp->size));
            bp=(blkHdr *)((char *)bp+PSIZE(bp->size));
          }
      }
    else
      {
        int i;
        size_t *blkFtr=(size_t *)((char *)blk-ALIGN(sizeof(size_t)));
        for(i=0;i<8;i++)
          {
            blk=(void *)((char *)blk-PSIZE(*blkFtr));
            blkFtr=(size_t *)((char *)blk-ALIGN(sizeof(size_t)));
          }
        blkHdr *bp=(blkHdr *)blk;
        for(i=0;i<8;i++)
          {

            printf("%s blk at %p, size %d\n",(bp->size&1)?"allocated":"free",bp,PSIZE((int)bp->size));
            bp=(blkHdr *)((char *)bp+PSIZE(bp->size));

          }
      }
  }


