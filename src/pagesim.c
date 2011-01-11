/*
 * pagesim.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#include <errno.h>
#include "pagesim.h"

/*tablica stron*/
page* pageTable;

void initPage(page* newPage, unsigned page_size){
   newPage->properties = 0;
   newPage->counter = 0;
   newPage->frameAddr = 0; /*nie przydzielam niczego*/
}

int page_sim_init(unsigned page_size, 
                         unsigned mem_size,
                         unsigned addr_space_size,
                         unsigned max_concurrent_operations,
                         pagesim_callback callback){
   unsigned i;
   
   /*zajmowanie zasobów*/
   pageTable = malloc(sizeof (page) * addr_space_size); /*obsuga bledu*/
   for(i = 0; i < addr_space_size; ++i)
      initPage(page + i, page_size);
   
   errno = EACCES;
   return -1;
}

int page_sim_get(unsigned a, uint8_t *v){
   return -1;
}
int page_sim_set(unsigned a, uint8_t v){
   return -1;
}
