/*
 * pagesim.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#define VERBOSE 1
#include <stdio.h>
#if VERBOSE == 1
int verbose = 1;
#else
int verbose = 0;
#endif

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "page.h"
#include "pagesim.h"
#include "strategy.h"

/*zmienne globalne*/
/*tablica stron*/
page* pages = 0;
/*sumylowana pamięć operacyjna*/
uint8_t* sim_memory = NULL;
/*deskryptor pliku stron na dysku*/
int pagefileFD = 0;

#define PAGEFILENAME "./pagefile"


/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0};

void initPage(page* newPage, unsigned page_size){
   newPage->properties = 0;
   newPage->counter = 0;
   newPage->frame = NULL;
} /*initPage*/

#define check_param ( \
       (sim_p.mem_size <= sim_p.addr_space_size)\
    && (sim_p.page_size >= MINPAGESIZE)\
    && (sim_p.page_size <= MAXPAGESIZE)\
    && (sim_p.mem_size >= MINMEMSIZE)\
    && (sim_p.mem_size <= MAXMEMSIZE)\
    && (sim_p.addr_space_size >= MINADDRSPACESIZE)\
    && (sim_p.addr_space_size <= MAXADDRSPACESIZE)\
    )

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
   sim_p.offsetMask = sim_p.page_size - 1;
   
   unsigned x = 2; /*log_2(page_size)*/
   sim_p.shift = 1;
   while (x < sim_p.page_size){
      x <<= 1;
      sim_p.shift++;
   }
   
   if (verbose) fprintf(stderr, "DEBUG: "
      "page_sim_init(): "
      "\n\t->pagesize=%#x"
      "\n\t->sim_offsetMask=%#x; "
      "\n\t->sim_p.shift=%#x\n",sim_p.page_size ,sim_p.offsetMask, sim_p.shift);
   
   /*sprawdzenie poprawności wprowadzonych danych*/
   if (!check_param){
      /*dane są niepoprawne*/
      errno = EINVAL;
      return -1;
   }
   
   /*zajmowanie zasobów*/
   unsigned i;
   pages = malloc(sizeof (page) * addr_space_size);
   for(i = 0; i < addr_space_size; ++i)
      initPage(pages + i, page_size);
   sim_memory = malloc(sim_p.mem_size * sim_p.page_size);
   
   /*tworzenie pliku*/
   pagefileFD = creat(PAGEFILENAME, 0644);
   if(pagefileFD == -1)
      return -1;
   
   return 0;
} /*page_sim_init*/

extern int page_sim_end(){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_end(): rozpoczecie\n");
   unlink(PAGEFILENAME);
   free(sim_memory);
   free(pages);
   if (verbose) fprintf(stderr, "DEBUG: page_sim_end(): zakonczenie\n");
   return 0;
} /*page_sim_end()*/

/*define liczące numer strony*/
/* i offset z adresu wirtualnego */
#define PAGENR(ADDR) ((ADDR) >> sim_p.shift)
#define OFFSET(ADDR) (ADDR & sim_p.offsetMask)

int page_sim_get(unsigned a, uint8_t *v){
   /*sprawdzanie czy zainicjowane!, czy nie wyrzucone*/
   sim_p.callback(1, PAGENR(a), OFFSET(a));
   if((pages[PAGENR(a)].properties) & VBIT){
      /*strona jest w pamieci*/
      if (verbose) fprintf(stderr, "DEBUG: page_sim_get(): Strona jest w pamięci, odczytuję\n");
      *v = pages[PAGENR(a)].frame[OFFSET(a)];
   } else {
      if (verbose) fprintf(stderr, "DEBUG: page_sim_get(): Nie ma strony w pamięci\n");
      /*stronę nalezy załadować z dysku*/
      
      /*jezeli nie ma wolnej ramki to trzeba zwolnić srtonę*/
   }
   /*tutaj strona powinna być w pamięci*/
   /*trzeba zająć się synchronizacją bo wchodzi*/
   /*czytelnik*/
   
   return 0;
} /*page_sim_get*/
unsigned firstFreeFrame = 0;
unsigned counter = 0;
int page_sim_set(unsigned a, uint8_t v){
   sim_p.callback(1, PAGENR(a), OFFSET(a));
   if((pages[PAGENR(a)].properties) & VBIT){
      pages[PAGENR(a)].frame[OFFSET(a)] = v;
   } else {
      if (verbose) fprintf(stderr, "DEBUG: page_sim_set(): nie mam takiej strony, tworzę\n");
      pages[PAGENR(a)].properties |= VBIT;
      pages[PAGENR(a)].frame = sim_memory + firstFreeFrame;
      firstFreeFrame++;
      pages[PAGENR(a)].frame[OFFSET(a)] = v;
   }
   /*podobnie jak wyżej*/
   /*pisarz na danej stronie*/
   
   return 0;
} /*page_sim_set*/
