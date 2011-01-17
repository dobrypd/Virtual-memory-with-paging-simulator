/*
 * strategyLFU.c
 * 
 * author: Piotr Dobrowolski
 * 11.01.2011r.
 *
*/
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "page.h"
#include "strategy.h"

#define DEBUG 1


page* select_page(page* pages, size_t size){
   #if DEBUG == 1
   fprintf (stderr, "/***************select_page*************************/\n");
   #endif
   unsigned i;
   page* minCounterPageModified = NULL;
   page* minCounterPageUnmodified = NULL;
   unsigned long long minM = ULLONG_MAX, minUM = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      #if DEBUG == 1
      fprintf(stderr, "Page nr: %d (ADDR = %p), properties: %u, frame addres: %p, counter: %llu\n", i, pages + i, pages[i].properties, pages[i].frame, pages[i].counter);
      #endif
      if (VPAGE(pages[i])){
         if(MPAGE(pages[i])){
            if (pages[i].counter < minM){
               minM = pages[i].counter;
               minCounterPageModified = pages + i;
            }
         } else {
            if (pages[i].counter < minUM){
               minUM = pages[i].counter;
               minCounterPageUnmodified = pages + i;
            }
         }
      }
   }
   #if DEBUG == 1
   fprintf(stderr, "SELECTED: MOD%p \tUMOD%p\n", minCounterPageModified, minCounterPageUnmodified);
   fprintf (stderr, "/***************************************************/\n");
   #endif
   if ((minUM == ULLONG_MAX) && (minM == ULLONG_MAX)) return NULL;
   if (minUM <= minM) return minCounterPageUnmodified;
   return minCounterPageModified;
}

int touch_page(page* tpage, size_t size){
   #if DEBUG == 1
   fprintf (stderr, "touch page\n");
   #endif
   tpage->counter++;
   return 0;
}