/*
 * strategy.h
 * 
 * author: Piotr Dobrowolski
 * 4.01.2011r.
 *
*/

#ifndef __STRATEGY_H
#define __STRATEGY_H

#include <stdlib.h>

#include "page.h"
extern page* select_page(page* pages, size_t size);

extern int touch_page(page* tpage, size_t size);

#endif
