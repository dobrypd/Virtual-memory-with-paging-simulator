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
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <aio.h>
#include <pthread.h>

#include "err.h"
#include "page.h"
#include "pagesim.h"
#include "strategy.h"

#define PAGEFILENAME "pagefileXXXXXX"
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
char pagefile_name[1024];
/*struktury aiocb dla każdej strony*/
struct aiocb* aiocb_for_frame = NULL;
/*ile operacji aktualnie*/
unsigned current_operations = 0;

/*parametry symulacji*/
pageSimParam_t sim_p = {0,0,0,0,0,0,0,0,0};

/*synchronizacja*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t io_limit = PTHREAD_COND_INITIALIZER;
pthread_cond_t in_use = PTHREAD_COND_INITIALIZER;

/*-------------deklaracje--------------*/
void initPage(page* newPage);
int page_sim_init(unsigned page_size, 
      unsigned mem_size, unsigned addr_space_size,
      unsigned max_concurrent_operations,
      pagesim_callback callback);
int page_sim_end();
uint8_t* alloc(unsigned page_nr);
int rw_page(page* wp, unsigned nr, unsigned OPTYPE);
int load_page(unsigned page_nr);
int check_addr(unsigned a);
int page_sim_get(unsigned a, uint8_t *v);
int page_sim_set(unsigned a, uint8_t v);
/***************************************/


void initPage(page* newPage){
   /*lock*/
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

void init_sim_p(unsigned page_size, 
                unsigned mem_size,
                unsigned addr_space_size,
                unsigned max_concurrent_operations,
                pagesim_callback callback){
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
}

void alloc_res(){
   unsigned i;
   pages = malloc(sizeof (page) * sim_p.addr_space_size);
   if (pages == NULL) fatal("pages malloc() error");
   for(i = 0; i < sim_p.addr_space_size; ++i)
      initPage(pages + i);
   sim_memory = malloc(sim_p.mem_size * sim_p.page_size);
   if (pages == NULL) fatal("pages malloc() error");
   aiocb_for_frame = malloc(sizeof(struct aiocb) * sim_p.mem_size);
   if (pages == NULL) fatal("pages malloc() error");
}

int page_sim_init(unsigned page_size, 
                         unsigned mem_size,
                         unsigned addr_space_size,
                         unsigned max_concurrent_operations,
                         pagesim_callback callback){
   /*zakładam że parametry są potęgami dwójki*/
   
   INITNR;
   
   if (verbose) fprintf(stderr, "DEBUG: page_sim_init():\n");
   
   SECURENR(pthread_mutex_lock(&mutex), locking mutex: page_sim_init():);
   
   if(sim_p.init != 0){
      errno = EPERM; /*niedozwolone jest wlaczenie dwoch w jednym procesie*/
      return -1;
   }
   
   init_sim_p(page_size, mem_size, addr_space_size, max_concurrent_operations, callback);
   
   /*sprawdzenie poprawności wprowadzonych danych*/
   if (!check_param){
      /*dane są niepoprawne*/
      errno = EINVAL;
      return -1;
   }
   
   /*zajmowanie zasobów*/
   alloc_res();     
   
   /*tworzenie pliku*/
   strcpy(pagefile_name, PAGEFILENAME);
   SECURE(pagefileFD = mkstemp(pagefile_name), mkstemp);
   
   /*inicjowanie co sie da w aiocb*/
   unsigned i;
   for(i = 0; i < sim_p.mem_size; ++i){
      aiocb_for_frame[i].aio_fildes = pagefileFD;
      aiocb_for_frame[i].aio_offset = 0x0;
      aiocb_for_frame[i].aio_nbytes = sim_p.page_size;
      aiocb_for_frame[i].aio_buf = sim_memory + sim_p.page_size*i;
      aiocb_for_frame[i].aio_lio_opcode = LIO_NOP;
      aiocb_for_frame[i].aio_sigevent.sigev_notify = SIGEV_NONE;
   }
   
   current_operations = 0;
   sim_p.init = 1;
   
   SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: page_sim_init():);
   return 0;
} /*page_sim_init*/



int page_sim_end(){
   if (verbose) fprintf(stderr, "DEBUG: page_sim_end():\n");
   
   INITNR;
   SECURENR(pthread_mutex_lock(&mutex), locking mutex: page_sim_end():);
   
   SECURE(close(pagefileFD), closing file: );
   /*unlink(PAGEFILENAME);*/
   free(sim_memory);
   free(pages);
   free(aiocb_for_frame);
   sim_p.init = 0;
   sim_p.page_size = 0;
   sim_p.mem_size = 0;
   sim_p.addr_space_size = 0;
   sim_p.max_concurrent_operations = 0;
   sim_p.callback = 0;
   sim_p.shift = 0;
   sim_p.offsetMask = 0;   
   sim_p.fff = 0;

   sim_memory = NULL;
   pages = NULL;
   aiocb_for_frame = NULL;
   
   SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: page_sim_end():);
   
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


/*OPTYPE 1-write 0-read*/
int rw_page(page* pg, unsigned nr, unsigned OPTYPE){
   sim_p.callback(2, nr, FRAMENR(pg->frame)); /*inicjacja*/
   INITNR;
   
   while(current_operations >= sim_p.max_concurrent_operations){
      SECURENR(pthread_cond_wait(&io_limit, &mutex), pthread cond wait in rw_page);
   }
   
   if(!sim_p.init){
      errno = 1010202; /*TODO:ENOINIT*/
      return -1;
   }
   
   current_operations++;
   
   SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex in read/write page:);
   
   /*ZAPIS / ODCZYT*/
   aiocb_for_frame[FRAMENR(pg->frame)].aio_offset = nr * sim_p.page_size;
   if (verbose) fprintf(stderr, "\t\t\t-saving/reading offset: %u\n", nr * sim_p.page_size);
   
   if(OPTYPE == 1) {
      SECURE(aio_write(aiocb_for_frame + FRAMENR(pg->frame)), aio_write():);
   } else {
      SECURE(aio_read(aiocb_for_frame + FRAMENR(pg->frame)), aio_read():);
   }
   
   if (aio_error(aiocb_for_frame + FRAMENR(pg->frame)) == EINPROGRESS){
      if (verbose) fprintf(stderr, "\t\t\t-waiting...\n");
   }
   
   struct aiocb **aiocb_waiton = malloc(sizeof(struct aiocb*));
   aiocb_waiton[0] = aiocb_for_frame + FRAMENR(pg->frame);
   SECURE(aio_suspend((const struct aiocb *const *) aiocb_waiton, 1, NULL), aio_suspend():);
   if (VERBOSE) fprintf(stderr, "\t\t\t-dump: errno=%d\n", aio_error(aiocb_for_frame + FRAMENR(pg->frame)));
   free(aiocb_waiton);
   
   sim_p.callback(3, nr, FRAMENR(pg->frame)); /*zapisano*/
   SECURENR(pthread_mutex_lock(&mutex), mutex lock read/write:);
   current_operations--;
   SECURENR(pthread_cond_signal(&io_limit), cond signal read/write:);
   return 0;
}



int load_page(unsigned page_nr){
   if (verbose) fprintf(stderr, "\tload_page(%u):\n", page_nr);
   sim_p.callback(1, page_nr, 0); /*rozpoczecie odwolania do strony*/
   
   INITNR;
   
   while(pages[page_nr].properties & UBIT){
      SECURENR(pthread_cond_wait(&(in_use), &mutex), cond wait- page in use);
   }
   
   if(!sim_p.init){
      errno = 1010202; /*TODO:ENOINIT*/
      return -1;
   }
   
   /*sprawdzam czy jest w pamięci*/
   if(VPAGE(pages[page_nr])){
      if (verbose) fprintf(stderr, "\t-direct\n");
   } 
   
   
   while (!VPAGE(pages[page_nr])) { /*nie jest w pamieci*/
      
      /*sprawdzam czy zaalokować czy wczytać z dysku*/
      if(sim_p.fff < sim_p.mem_size){
         
         /*nie ma go w pamięci a pamięć jest jeszcze wolna*/
         /*ponieważ nie można zwalniać zasobów*/
         /*oznacza to że muszę to zaalokować*/
         if (verbose) fprintf(stderr, "\t-not initialized\n");
         
         pages[page_nr].frame = alloc(page_nr);
         
      } else {  /* swapping pages */
         
         if (verbose) fprintf(stderr, "\t-in pagefile\n");
         
         pages[page_nr].properties |= UBIT;
         
         page* to_change = select_page(pages, sim_p.addr_space_size);
         while(! to_change){
            SECURENR(pthread_cond_wait(&in_use, &mutex), waiting to page);
            to_change= select_page(pages, sim_p.addr_space_size);
         }
         if(!sim_p.init){
            errno = ECANCELED;
            return -1;
         }

         if (verbose) fprintf(stderr, "\t\t-selected page nr %lu with counter=%llu\n", (to_change - pages), to_change->counter);
         
         to_change->properties |= UBIT;
         /*jezeli zmodyfikowana*/
         if ((to_change->properties) & MBIT){
            SECURE(rw_page(to_change, (to_change - pages), 1), write_page():);
         }
         
         to_change->counter = 0;
         to_change->properties = 0;
         
         /*odczytuje strone*/
         pages[page_nr].frame = to_change->frame; /*na ten adres*/
         SECURE(rw_page(pages + page_nr, page_nr, 0), read_page():);
         
         to_change->properties &= ~UBIT;
         
         pages[page_nr].properties |= VBIT;
         pages[page_nr].properties &= ~MBIT;
      } /* else swapping */
   } /*VPAGE check*/
   
   touch_page(pages + page_nr, sim_p.addr_space_size);
   
   pages[page_nr].properties &= ~UBIT;
   SECURENR(pthread_cond_broadcast(&in_use), broadcast);
   
   sim_p.callback(6, page_nr, 0); /*wykonano*/
   if (verbose) fprintf(stderr, "\tloaded\n");
   
   return 0;
} /*load_page*/

int check_addr(unsigned a){
   if (a > (sim_p.addr_space_size*sim_p.page_size - 1))
      return -1;
   return 0;
}

int page_sim_get(unsigned a, uint8_t *v){
   INITNR;
   if (verbose) fprintf(stderr, "DEBUG: page_sim_get(%#x, &): \n", a);
   SECURENR(pthread_mutex_lock(&mutex), locking mutex: page_sim_get():);
   if(!sim_p.init){
      if (verbose) fprintf(stderr, "SIMULATOR NOT INITIALIZED!\n");
      SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: while error():);
      return -1;
   }
   
   if(check_addr(a)){
      if (verbose) fprintf(stderr, "Addres incorrect\n");
      SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: while error():);
      return -2;
   }
   
   SECURE(load_page(PAGENR(a)), load_page());
   
   *v = pages[PAGENR(a)].frame[OFFSET(a)];
   
   SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: page_sim_get():);
   
   if (verbose) fprintf(stderr, "->OK\n");
   
   return 0;
} /*page_sim_get*/



int page_sim_set(unsigned a, uint8_t v){
   INITNR;
   if (verbose) fprintf(stderr, "DEBUG: page_sim_set(%#x, %u): \n", a, v);
   SECURENR(pthread_mutex_lock(&mutex), locking mutex: page_sim_set():);
   if(!sim_p.init){
      if (verbose) fprintf(stderr, "SIMULATOR NOT INITIALIZED!\n");
      SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: while error():);
      return -1;
   }
   
   if(check_addr(a)){
      if (verbose) fprintf(stderr, "Addres incorrect\n");
      SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: while error():);
      return -2;
   }
   
   SECURE(load_page(PAGENR(a)), load_page():);
   
   pages[PAGENR(a)].properties |= (pages[PAGENR(a)].frame[OFFSET(a)] == v)?0:MBIT;
   pages[PAGENR(a)].frame[OFFSET(a)] = v;
   
   SECURENR(pthread_mutex_unlock(&mutex), unlocking mutex: page_sim_set():);
   
   if (verbose) fprintf(stderr, "->OK\n");
   
   return 0;
} /*page_sim_set*/
