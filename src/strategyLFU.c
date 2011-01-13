/*
 * strategyLFU.c
 * 
 * author: Piotr Dobrowolski
 * 11.01.2011r.
 *
*/
#include <limits.h>
#include <stdlib.h>
#include "page.h"
#include "strategy.h"

page* select_page(page* pages, size_t size){
   unsigned i;
   page* minCounterPageModified;
   page* minCounterPageUnmodified;
   unsigned long long minM = ULLONG_MAX, minUM = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      if (pages[i].properties & VBIT){
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
   return (minM==minUM)?(minCounterPageUnmodified):(minCounterPageModified);
}

int touch_page(page* tpage){
   tpage->counter++;
   return 0;
}