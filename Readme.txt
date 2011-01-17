/*
 * Readme.txt
 * 
 * author: Piotr Dobrowolski
 * 17.01.2011r.
 *
*/

Opis rozwiązania: (w tym użytych metod komunikacji i synchronizacji)


Do synchronizacji stosuję
   - mutex - do ochrony zmiennych globalnych jeżeli dana funkcja potrzebuje ochrony mutexem podczas wywołania to jest to zaznaczone w komentarzu przed definicją funkcji.
   - io_limit - warunek na który czekają wątki które nie mogą zrobić zapisu / odczytu na dysk ponieważ jest już takich wystarczająco dużo (podawane przy page_sim_init)
   - in_use - warunek na który czekają wątki które chcą uzyć strony, a która jest aktualnie wykorzystywana przez inny wątek, jeżeli jakaś strona skończy używać to robi signal - broadcast.
   
Najważniejsze funkcje:
   - page_sim_sg: dla set i get: blokuje mutex, i wywołuje load_page aby upewnić się że strona istnieje / pobrać stronę z dysku. Funkcja później (trzyma mutex) zapisuje / odczytuje wartość. Po wykonaniu operacji na stronie wykonuję brodcast na czekających na stronę.
   - load_page: funkcja odpowiedzialna za pobranie strony do pamięci sim_memory. Początkowo może się zawiesić na warunku, jeżeli dana strona jest w użyciu przez inny proces (UBIT). Jeżeli nie jest, to dopóki strona nie znajdzie się w pamięci próbuje ją pobrać. Najpierw sprawdzane jest czy może pamięć sim_memory nie jest niezapełniona, alokuje. Jeżeli należy zamienić strony to odpalane jest page_select, dopóki strona nie zostanie znaleziona (czekanie na in_use). Zeruję znalezionej stronie VBIT, po to aby uniknąć ponownego jej wybrania przez select_page w innym wątku. Zostaje ustawiony bin UBIT, blokuję stronę w ten sposób. Jeżeli strona została zmieniona to chcę ją zapisać na dysk - wywołuję rw_page. Zeruję informacje o stronie zapisanej na dysk (oprócz UBIT). Następnie pobieram w to miejsce w pamięci sim_memory stronę z dysku. Po operacji na dysku zmieniam UBIT - odblokowuje stronę zapisaną na dysk. Ustawiam VBIT - bit obecności w pamięci. Następnie - dla każdej wywołanej strony, niezależnie czy już była w pamięci czy nie wywołuje touch_page i zaznaczam że już nie będę korzystał ze strony.
   - rw_page: funkcja służy do obsługi operacji aio. Na początku sprawdzam ilość wykonywanych aktualnie operacji, jeżeli jest za duża to czekam na io_limit. Odblokowuję mutexa i przechodzę do zapisania / odczytu strony. Następnie czekam na wykonanie zadnia, a później na mutexa.
   
Strategie:
   FIFO i LFU: zeruję licznik przy zapisaniu na dysk
   LFU: zliczam od momentu załadowania do symulowanej pamięci fizycznej
   do aktualizowania informacji o stronie wykonuję touch_page
      
Biblioteka używa do komunikowania się zmiennych globalnych.

Aby złączyć bibliotekę należy odpalić make fifo / make lfu, oraz linkować program wg schematu:
gcc simpleTest.c -o test -L. -lpagesim
jeżeli pagesim.so jest w tym samym katalogu to wywołać:
LD_LIBRARY_PATH=.
export LD_LIBRARY_PATH