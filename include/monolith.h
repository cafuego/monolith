/* $Id$ */
/* #define DEBUG_MEMORY_LEAK */

#include "sysconfig.h"
#include "defs.h"
#include "global.h"
#include "typedefs.h"
#include "version.h"

int cprintf( const char *format, ... );

#ifdef DEBUG_MEMORY_LEAK
int allocated_ctr;   /* incremented in xmalloc, decremented in xfree */
unsigned long allocated_total;  /* total bytes malloc'd at present */
#endif

