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
page* pages = 0;
/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0};

void initPage(page* newPage, unsigned page_size){
   newPage->properties = 0;
   newPage->counter = 0;
   newPage->frame = NULL; /*nie przydzielam niczego*/
} /*initPage*/

int check_param(){
   unsigned char ret = !0;
   ret = (sim_p.mem_size <= sim_p.addr_space_size);
   ret &= (sim_p.page_size >= MINPAGESIZE);
   ret &= (sim_p.page_size <= MAXPAGESIZE);
   ret &= (sim_p.mem_size >= MINMEMSIZE);
   ret &= (sim_p.mem_size <= MAXMEMSIZE);
   ret &= (sim_p.addr_space_size >= MINADDRSPACESIZE);
   ret &= (sim_p.addr_space_size <= MAXADDRSPACESIZE);
   
   return ret;
} /*check_param*/

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
   sim_p.pagenrMask = (sim_p.page_size * sim_p.addr_space_size -1);
   sim_p.pagenrMask &= sim_p.offsetMask;
   /*sprawdzenie poprawności wprowadzonych danych*/
   if (!check_param()){
      /*dane są niepoprawne*/
      errno = EINVAL;
      return -1;
   }
   
   /*zajmowanie zasobów*/
   unsigned i;
   pages = malloc(sizeof (page) * addr_space_size); /*obsuga bledu*/
   for(i = 0; i < addr_space_size; ++i)
      initPage(pages + i, page_size);
   
   return 0;
} /*page_sim_init*/

extern int page_sim_end(){
      /*przejdź po stronach i pousuwaj ramki*/
      free(pages);
} /*page_sim_end()*/

/*define liczące numer strony*/
/* i offset z adresu wirtualnego */
#define PAGENR(ADDR) (ADDR & sim_p.pagenrMask)
#define OFFSET(ADDR) (ADDR & sim_p.offsetMask)

int page_sim_get(unsigned a, uint8_t *v){
   sim_p.callback(1, PAGENR(a), 0);
   page* cur_page = NULL;
   if((pages[PAGENR(a)].properties) & VBIT){
      /*strona jest w pamieci*/
   } else {
      /*stronę nalezy załadować z dysku*/
      
      /*jezeli nie ma wolnej ramki to trzeba zwolnić srtonę*/
   }
   /*tutaj strona powinna być w pamięci*/
   /*trzeba zająć się synchronizacją bo wchodzi*/
   /*czytelnik*/
   v = page[PAGENR(a)].frame[OFFSET(a)];
   
   return 0;
} /*page_sim_get*/

int page_sim_set(unsigned a, uint8_t v){
   sim_p.callback(1, PAGENR(a), 0);
   
   /*podobnie jak wyżej*/
   /*pisarz na danej stronie*/
   
   return 0;
} /*page_sim_set*/
