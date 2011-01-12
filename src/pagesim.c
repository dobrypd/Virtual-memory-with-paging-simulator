/*
 * pagesim.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#define VERBOSE 1
#if VERBOSE == 1
#include <stdio.h>
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


/*define liczące numer strony*/
/* i offset z adresu wirtualnego */
#define PAGENR(ADDR) ((ADDR) >> sim_p.shift)
#define OFFSET(ADDR) (ADDR & sim_p.offsetMask)

#define PAGEFILENAME "./pagefile"


/*zmienne globalne*/
/*tablica stron*/
page** pages = 0;
/*sumylowana pamięć operacyjna*/
uint8_t* sim_memory = NULL;
/*deskryptor pliku stron na dysku*/
int pagefileFD = 0;


/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0};


unsigned alloc(unsigned page_nr){
   if (verbose) fprintf(stderr, 
      "DEBUG: alloc(): alokuje nowy obszar (nr %u) o adresie %#x\n", 
                        sim_p.fff, sim_memory + sim_p.page_size*(sim_p.fff));
   pages[page_nr]->properties |= MBIT;
   
   return (sim_memory + sim_p.page_size*sim_p.fff++);
} /*alloc*/


page* initPage(unsigned frame_addr){
   page* newPage = malloc(sizeof(page));
   newPage->properties = 0;
   newPage->counter = 0;
   newPage->frame = NULL;
   pages[page_nr]->frame = frame_addr;
   return newPage;
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
   pages = malloc(addr_space_size * sizeof(page*));
   sim_memory = malloc(sim_p.mem_size * sim_p.page_size);
   for(i = 0; i < addr_space_size; ++i)
      pages[i] = NULL;
   /*tworzenie pliku*/
   pagefileFD = creat(PAGEFILENAME, 0644);
   if(pagefileFD == -1)
      return -1;
   
   return 0;
} /*page_sim_init*/

extern int page_sim_end(){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_end(): \n");
   
   unlink(PAGEFILENAME);
   
   free(sim_memory);
   
   unsigned i;
   for(i = 0; i < sim_p.addr_space_size; ++i)
      if(pages[i] != NULL)
         free(pages[i]);
   free(pages);
   
   if (verbose) fprintf(stderr, "OK\n");
   return 0;
} /*page_sim_end()*/

int load_page(unsigned page_nr){
   if (verbose) fprintf(stderr, "\tload_page(%u):\n", page_nr);
   
   /*sprawdzam czy jest w pamięci*/
   if(pages[page_nr] != NULL){
      if (verbose) fprintf(stderr, "\t-direct\n");
      
   } else { /*nie jest w pamieci*/
      
      /*sprawdzam czy zaalokować czy wczytać z dysku*/
      if(sim_p.fff <= sim_p.mem_size){
         
         /*nie ma go w pamięci a pamięć jest jeszcze wolna*/
         /*ponieważ nie można zwalniać zasobów*/
         /*oznacza to że muszę to zaalokować*/
         if (verbose) fprintf(stderr, "\t-not initialized\n");
         pages[page_nr]       = initPage(alloc(page_nr));
         
      } else {
         
         /*tu się zaczyna cała zabawa:*/
         /*nie mam strony w pamięci   */
         /*i nie mam miejsca na nią   */
         /*trzeba coś wywalić na dysk */
         if (verbose) fprintf(stderr, "\t-in pagefile\n");
         
         page* to_change = select_page(pages, sim_p.addr_space_size);
         if (to_change == NULL){
            /*błąd?*/
         }
         
         /*    wyrzucam strone z pamięci (NR)
          *       (to_change->frame - sim_memory) / sim_p.page_size
          *    na dysk adres (w pliku dyskowym pagefile)
          *       (to_change - pages) / sizeof(page)
          */
         sim_p.callback(2, (to_change - pages) / sizeof(page*), (to_change->frame - sim_memory) / sim_p.page_size); /*inicjacja*/
         
         /*ZAPIS*/
         
         sim_p.callback(3, (to_change - pages) / sizeof(page*), (to_change->frame - sim_memory) / sim_p.page_size); /*zapisano*/
         
         /*    wczytuję stronę z dysku z adresu
          *       page_nr
          *    do pamieci pod adres
          *       (to_change.frame - sim_memory) / sim_p.page_size
          */
         sim_p.callback(4, page_nr, (to_change->frame - sim_memory) / sim_p.page_size); /*inicjowane*/
         
         /*WCZYTYWANIE*/
         
         sim_p.callback(5, page_nr, (to_change->frame - sim_memory) / sim_p.page_size); /*wczytano*/
         pages[page_nr] = initPage;
         pages[page_nr].frame = to_change->frame;
         
      }
   }
   if (verbose) fprintf(stderr, "\tloaded\n");
   return 0;
} /*load_page*/

int page_sim_get(unsigned a, uint8_t *v){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_get(%#x, &): \n", a);
   
   sim_p.callback(1, PAGENR(a), OFFSET(a)); /*rozpoczenie odwolania do strony*/
   
   load_page(PAGENR(a));
   
   *v = pages[PAGENR(a)].frame[OFFSET(a)];
   
   sim_p.callback(1, PAGENR(a), (pages[PAGENR(a)].frame - mem_size) / sim_p.page_size); /*wykonano*/
   
   if (verbose) fprintf(stderr, "\t\t->OK\n");
   
   return 0;
} /*page_sim_get*/

int page_sim_set(unsigned a, uint8_t v){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_set(%#x, %u): \n", a, v);
   
   sim_p.callback(1, PAGENR(a), OFFSET(a)); /*rozpoczecie odwolania do strony*/
   
   load_page(PAGENR(a));
   
   pages[PAGENR(a)].properties |= (pages[PAGENR(a)].frame[OFFSET(a)] == v)?0:MBIT;
   pages[PAGENR(a)].frame[OFFSET(a)] = v;
   
   sim_p.callback(1, PAGENR(a), (pages[PAGENR(a)].frame - mem_size) / sim_p.page_size); /*wykonano*/
   
   if (verbose) fprintf(stderr, "\t\t->OK\n");
   
   return 0;
} /*page_sim_set*/
