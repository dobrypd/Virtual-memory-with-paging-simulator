/*
 * pagesim.c
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/
#include <stdio.h>
#define VERBOSE 1
#if VERBOSE == 1
int verbose = 1;
#else
int verbose = 0;
#endif

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <aio.h>

#include "err.h"
#include "page.h"
#include "pagesim.h"
#include "strategy.h"

#define PAGEFILENAME "./pagefile"
/*define liczące numer strony*/
/* i offset z adresu wirtualnego */
#define PAGENR(ADDR) ((ADDR) >> sim_p.shift)
#define OFFSET(ADDR) ((ADDR) & sim_p.offsetMask)
#define FRAMENR(ADDR) (((ADDR) - sim_memory) / sim_p.page_size)


/*zmienne globalne*/
/*tablica stron*/
page* pages = 0;
/*sumylowana pamięć operacyjna*/
uint8_t* sim_memory = NULL;
/*deskryptor pliku stron na dysku*/
int pagefileFD = 0;
/*struktury aiocb dla każdej strony*/
struct aiocb* aiocb_for_frame = NULL;


/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0,0,0};

/*-------------deklaracje--------------*/
void initPage(page* newPage);
int page_sim_init(unsigned page_size, 
      unsigned mem_size, unsigned addr_space_size,
      unsigned max_concurrent_operations,
      pagesim_callback callback);
int page_sim_end();
uint8_t* alloc(unsigned page_nr);
int write_page(page* wp, unsigned nr);
int read_page(page* rp, unsigned nr);
int load_page(unsigned page_nr);
int page_sim_get(unsigned a, uint8_t *v);
int page_sim_set(unsigned a, uint8_t v);
/***************************************/


void initPage(page* newPage){
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
   sim_p.fff = 0;
   
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
      /*errno = EINVAL;
      return -1;*/
   }
   
   /*zajmowanie zasobów*/
   unsigned i;
   pages = malloc(sizeof (page) * sim_p.addr_space_size);
   if (pages == NULL) fatal("pages malloc() error");
   for(i = 0; i < sim_p.addr_space_size; ++i)
      initPage(pages + i);
   sim_memory = malloc(sim_p.mem_size * sim_p.page_size);
   if (pages == NULL) fatal("pages malloc() error");
   aiocb_for_frame = malloc(sizeof(struct aiocb) * sim_p.mem_size);
   if (pages == NULL) fatal("pages malloc() error");
      
   
   /*tworzenie pliku*/
   SECURE(pagefileFD = open(PAGEFILENAME, O_RDWR|O_CREAT,0644), creating file open():);
   SECURE(truncate(PAGEFILENAME, sim_p.addr_space_size * sim_p.page_size), truncate file);   
   
   /*inicjowanie co sie da w aiocb*/
   for(i = 0; i < sim_p.mem_size; ++i){
      aiocb_for_frame[i].aio_fildes = pagefileFD;
      aiocb_for_frame[i].aio_offset = 0x0; /*do usupełnienia (nr strony)*/
      aiocb_for_frame[i].aio_nbytes = sim_p.page_size * sizeof(uint8_t);
      aiocb_for_frame[i].aio_buf = sim_memory + i;
      aiocb_for_frame[i].aio_lio_opcode = LIO_NOP;
      aiocb_for_frame[i].aio_sigevent.sigev_notify = SIGEV_NONE;
   }
   
   sim_p.init = 1;
   return 0;
} /*page_sim_init*/



int page_sim_end(){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_end():\n");
   SECURE(close(pagefileFD), closing file: );
   unlink(PAGEFILENAME);
   free(sim_memory);
   free(pages);
   free(aiocb_for_frame);
   sim_p.init = 0;
   if (verbose) fprintf(stderr, "OK\n");
   return 0;
} /*page_sim_end()*/



uint8_t* alloc(unsigned page_nr){
   if (verbose) fprintf(stderr, 
      "\t\tDEBUG: alloc(): alokuje nowy obszar (nr %u) o adresie %p\n", 
                        sim_p.fff, sim_memory + sim_p.page_size*(sim_p.fff));
   
   pages[page_nr].properties |= MBIT;
   pages[page_nr].properties |= VBIT;
   if (verbose) fprintf(stderr, "\t\tDEBUG: alloc(): set properties: (pattern)= %#x\n", pages[page_nr].properties);
   
   return (sim_memory + sim_p.page_size*sim_p.fff++);
} /*alloc*/



int write_page(page* wp, unsigned nr){
   sim_p.callback(2, nr, FRAMENR(wp->frame)); /*inicjacja*/
   
   /*ZAPIS*/
   aiocb_for_frame[FRAMENR(wp->frame)].aio_offset = nr * sim_p.page_size;
   if (verbose) fprintf(stderr, "\t\t\t-saving offset: %u\n", nr * sim_p.page_size);
   
   SECURE(aio_write(aiocb_for_frame + FRAMENR(wp->frame)), aio_write():);
   
   if (aio_error(aiocb_for_frame + FRAMENR(wp->frame)) == EINPROGRESS){
      if (verbose) fprintf(stderr, "\t\t\t-waiting...\n");
   }
   
   struct aiocb **aiocb_waiton = malloc(sizeof(struct aiocb*));
   aiocb_waiton[0] = aiocb_for_frame + FRAMENR(wp->frame);
   SECURE(aio_suspend((const struct aiocb *const *) aiocb_waiton, 1, NULL), aio_suspend():);
   if (VERBOSE) printf("\t\t\t-dump: errno=%d\n", aio_error(aiocb_for_frame + FRAMENR(wp->frame)));
   free(aiocb_waiton);
   
   sim_p.callback(3, nr, FRAMENR(wp->frame)); /*zapisano*/
   return 0;
}



int read_page(page* rp, unsigned nr){
   sim_p.callback(4, nr, FRAMENR(rp->frame)); /*inicjowane*/
   
   /*WCZYTYWANIE*/  
   aiocb_for_frame[FRAMENR(rp->frame)].aio_offset = nr * sim_p.page_size;
   if (verbose) fprintf(stderr, "\t\t\t-reading offset: %u\n", nr * sim_p.page_size);
   
   SECURE(aio_read(aiocb_for_frame + FRAMENR(rp->frame)), aio_read():);
   
   if (aio_error(aiocb_for_frame + FRAMENR(rp->frame)) == EINPROGRESS){
      if (verbose) fprintf(stderr, "\t\t\t-waiting...\n");
   }
   
   struct aiocb **aiocb_waiton = malloc(sizeof(struct aiocb*));
   aiocb_waiton[0] = aiocb_for_frame + FRAMENR(rp->frame);
   SECURE(aio_suspend((const struct aiocb *const *) aiocb_waiton, 1, NULL), aio_suspend():);
   if (VERBOSE) printf("\t\t\t-dump: errno=%d\n", aio_error(aiocb_for_frame + FRAMENR(rp->frame)));
   free(aiocb_waiton);
   
   rp->properties |= VBIT;
   
   sim_p.callback(5, nr, FRAMENR(rp->frame)); /*wczytano*/
   return 0;
}



int load_page(unsigned page_nr){
   if (verbose) fprintf(stderr, "\tload_page(%u):\n", page_nr);
   
   /*sprawdzam czy jest w pamięci*/
   if(VPAGE(pages[page_nr])){
      if (verbose) fprintf(stderr, "\t-direct\n");
   } else { /*nie jest w pamieci*/
      
      /*sprawdzam czy zaalokować czy wczytać z dysku*/
      if(sim_p.fff < sim_p.mem_size){
         
         /*nie ma go w pamięci a pamięć jest jeszcze wolna*/
         /*ponieważ nie można zwalniać zasobów*/
         /*oznacza to że muszę to zaalokować*/
         if (verbose) fprintf(stderr, "\t-not initialized\n");
         pages[page_nr].frame = alloc(page_nr);
      } else {  /* swapping pages */
         
         if (verbose) fprintf(stderr, "\t-in pagefile\n");
         
         page* to_change = select_page(pages, sim_p.addr_space_size);
         if (to_change == NULL){
            if (verbose) fprintf(stderr, "\t\t-page does not found. Pointer to page is NULL!!!\n");
            errno = ECANCELED;
            return -1;
         }
         if (verbose) fprintf(stderr, "\t\t-selected page nr %lu with counter=%llu\n", (to_change - pages), to_change->counter);
         
         /*jezeli zmodyfikowana*/
         if (to_change->properties & MBIT){
            SECURE(write_page(to_change, (to_change - pages)), write_page():);
         } 
         
         to_change->counter = 0;
         to_change->properties = 0;
         
         /*odczytuje strone*/
         pages[page_nr].frame = to_change->frame; /*na ten adres*/
         SECURE(read_page(pages + page_nr, page_nr), read_page():);
      } /* else swapping */
   } /*VPAGE check*/
   
   touch_page(pages + page_nr);
   if (verbose) fprintf(stderr, "\tloaded\n");
   
   return 0;
} /*load_page*/



int page_sim_get(unsigned a, uint8_t *v){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_get(%#x, &): \n", a);
   if(!sim_p.init){
      if (verbose) fprintf(stderr, "SIMULATOR NOT INITIALIZED!\n");
      return -1;
   }
   
   sim_p.callback(1, PAGENR(a), OFFSET(a)); /*rozpoczenie odwolania do strony*/
   
   SECURE(load_page(PAGENR(a)), load_page());
   
   *v = pages[PAGENR(a)].frame[OFFSET(a)];
   
   sim_p.callback(6, PAGENR(a), FRAMENR(pages[PAGENR(a)].frame)); /*wykonano*/
   
   if (verbose) fprintf(stderr, "->OK\n");
   
   return 0;
} /*page_sim_get*/



int page_sim_set(unsigned a, uint8_t v){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_set(%#x, %u): \n", a, v);
   if(!sim_p.init){
      if (verbose) fprintf(stderr, "SIMULATOR NOT INITIALIZED!\n");
      return -1;
   }
   
   sim_p.callback(1, PAGENR(a), OFFSET(a)); /*rozpoczecie odwolania do strony*/
   
   SECURE(load_page(PAGENR(a)), load_page():);
   
   pages[PAGENR(a)].properties |= (pages[PAGENR(a)].frame[OFFSET(a)] == v)?0:MBIT;
   pages[PAGENR(a)].frame[OFFSET(a)] = v;
   
   sim_p.callback(6, PAGENR(a), FRAMENR(pages[PAGENR(a)].frame)); /*wykonano*/
   
   if (verbose) fprintf(stderr, "->OK\n");
   
   return 0;
} /*page_sim_set*/
