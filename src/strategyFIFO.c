/*
 * strategyFIFO.c
 * 
 * author: Piotr Dobrowolski
 * 11.01.2011r.
 *
*/
#include<limits.h>
#include "page.h"
#include "strategy.h"

page* select_page(page* pages, site_z size){
   unsigned i;
   page* minCounterPage;
   unsigned long long min = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      if (pages[i].counter < minM){
         minM = pages[i].counter;
         minCounterPageModified = page + i;
      }
   }
   return minCounterPageUnmodified;
}