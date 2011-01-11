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
page* pages                 = 0;
/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0};

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
   sim_p.page_size = page_size;
   sim_p.mem_size = mem_size;
   sim_p.addr_space_size = addr_space_size;
   sim_p.max_concurrent_operations = max_concurrent_operations;
   sim_p.callback = callback;
   sim_p.offsetMask = sim_p.page_size -1;
   sim_p.pagenrMask = (sim_p.page_size * sim_p.addr_space_size -1) & sim_p.offsetMask;
   
   /*zajmowanie zasobów*/
   unsigned i;
   pages = malloc(sizeof (page) * addr_space_size); /*obsuga bledu*/
   for(i = 0; i < addr_space_size; ++i)
      initPage(pages + i, page_size);
   
   errno = EACCES;
   return -1;
} /*page_sim_init*/

extern int page_sim_end(){
      /*przejdź po stronach i pousuwaj ramki*/
      free(pageTabl
} /*page_sim_end()*/

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
