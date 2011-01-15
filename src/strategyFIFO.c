/*
 * strategyFIFO.c
 * 
 * author: Piotr Dobrowolski
 * 11.01.2011r.
 *
*/
#include <stdlib.h>

#include <limits.h>
#include <stdlib.h>
#include "page.h"
#include "strategy.h"

unsigned long long counter = 1;

page* select_page(page* pages, size_t size){
   unsigned i;
   page* minCounterPage = NULL;
   unsigned long long min = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      printf("Strona nr: %d (ADDR = %#x), properties: %u, frame addres: %#x, counter: %llu\n", i, pages + i, pages[i].properties, pages[i].frame, pages[i].counter);
      if ((pages[i].properties & VBIT) && (pages[i].counter < min)){
         min = pages[i].counter;
         minCounterPage = pages + i;
      }
   }
   printf("SELECTED: %#x\n", minCounterPage);
   return minCounterPage;
}

int touch_page(page* tpage){
   if (tpage->counter != 0)
      return 0 ;
   tpage->counter = counter++;
   return 0;
}