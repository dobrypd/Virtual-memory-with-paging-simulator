/*
 * strategyFIFO.c
 * 
 * author: Piotr Dobrowolski
 * 11.01.2011r.
 *
*/
#include <limits.h>
#include <stdlib.h>
#include "page.h"
#include "strategy.h"

unsigned long long counter = 1;

page* select_page(page* pages, size_t size){
   unsigned i;
   page* minCounterPage;
   unsigned long long min = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      if ((pages[i].properties & VBIT) && pages[i].counter < min){
         min = pages[i].counter;
         minCounterPage = pages + i;
      }
   }
   return minCounterPage;
}

int touch_page(page* tpage){
   if (tpage->properties & VBIT){
      if (tpage->counter == 0){
         /*wchodzę z swapa do pamięci*/
      }
      tpage->counter = counter++;
   }
   return 0;
}