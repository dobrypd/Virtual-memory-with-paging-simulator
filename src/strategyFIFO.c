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

page* select_page(page* pages, size_t size){
   unsigned i;
   page* minCounterPage;
   unsigned long long min = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      if (pages[i].counter < min){
         min = pages[i].counter;
         minCounterPage = pages + i;
      }
   }
   return minCounterPage;
}