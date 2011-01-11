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

#define MODIF(X) (((X).properties) & 1)
page* select_page(page* pages, size_t size){
   unsigned i;
   page* minCounterPageModified;
   page* minCounterPageUnmodified;
   unsigned long long minM = ULLONG_MAX, minUM = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      if(MODIF(pages[i])){
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
   
   return (minM==minUM)?(minCounterPageUnmodified):(minCounterPageModified);
}