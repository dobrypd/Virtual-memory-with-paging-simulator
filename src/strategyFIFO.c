/*
 * strategyFIFO.c
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
unsigned long long counter = 1;

page* select_page(page* pages, size_t size){
   #if DEBUG == 1
   fprintf (stderr, "/***************select_page*************************/\n");
   #endif
   unsigned i;
   page* minCounterPage = NULL;
   unsigned long long min = ULLONG_MAX;
   for(i = 0; i < size; ++i){
      #if DEBUG == 1
      fprintf(stderr, "Page nr: %d (ADDR = %#x), properties: %u, frame addres: %#x, counter: %llu\n", i, pages + i, pages[i].properties, pages[i].frame, pages[i].counter);
      #endif
      if ((pages[i].properties & VBIT) && (pages[i].counter < min)){
         min = pages[i].counter;
         minCounterPage = pages + i;
      }
   }
   #if DEBUG == 1
   fprintf(stderr, "SELECTED: %#x\n", minCounterPage);
   fprintf (stderr, "/***************************************************/\n");
   #endif
   return minCounterPage;
}

int touch_page(page* tpage, size_t size){
   #if DEBUG == 1
   fprintf (stderr, "touch page\n");
   #endif
   if (tpage->counter != 0)
      return 0 ;
   tpage->counter = counter++;
   if (counter > 2*size){
      /*zawijaj*/
      /*counter = 1;*/
   }
   return 0;
}