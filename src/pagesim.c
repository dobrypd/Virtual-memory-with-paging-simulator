/*
 * pagesim.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#include <errno.h>
#include "page.h"
#include "pagesim.h"
#include "strategy.h"

/*zmienne globalne*/
/*tablica stron*/
page* pageTable             = 0;
pagesim_callback fcallback  = 0;
unsigned offsetMask         = 0;
unsigned pagenrMask         = 0;
unsigned Gaddr_space_size   = 0;

void initPage(page* newPage, unsigned page_size){
   newPage->properties = 0;
   newPage->counter = 0;
   newPage->frameAddr = 0; /*nie przydzielam niczego*/
} /*initPage*/

int page_sim_init(unsigned page_size, 
                         unsigned mem_size,
                         unsigned addr_space_size,
                         unsigned max_concurrent_operations,
                         pagesim_callback callback){
   /*zakładam że parametry są potęgami dwójki*/
   unsigned i;
   fcallback = callback;
   offsetMask = page_size -1;
   pagenrMask = (page_size * addr_space_size -1) & offsetMask;
   Gaddr_space_size = addr_space_size;
   
   /*zajmowanie zasobów*/
   pageTable = malloc(sizeof (page) * addr_space_size); /*obsuga bledu*/
   for(i = 0; i < addr_space_size; ++i)
      initPage(pageTable + i, page_size);
   
   errno = EACCES;
   return -1;
} /*page_sim_init*/

/*define liczące numer strony*/
/* i offset z adresu wirtualnego */
#define PAGENR(ADDR) (ADDR & pagenrMask)
#define OFFSET(ADDR) (ADDR & offsetMask)

int page_sim_get(unsigned a, uint8_t *v){
   fcallback(1, PAGENR(a), 0);
   select_page(pageTable, Gaddr_space_size);
   return -1;
} /*page_sim_get*/

int page_sim_set(unsigned a, uint8_t v){
   fcallback(1, PAGENR(a), 0);
   return -1;
} /*page_sim_set*/
