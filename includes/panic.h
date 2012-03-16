#ifndef _PANIC_H
#define _PANIC_H

#include <stdlib.h>
#include "log.h"

#define safemalloc(variable,type) do{ variable = ( type *)malloc(sizeof( type ));\
if(! variable ){\
panic_message("Failed to allocate for " #variable ".\n");\
}\
} while(0);

#define safenmalloc(variable,type,size) do{ variable = ( type *)malloc(sizeof( type )* size );\
if(! variable ){\
panic_message("Failed to allocate for " #variable ".\n");\
}\
} while(0);

#define safecpy(dest,src) do{\
safenmalloc( dest , char, strlen( src )+1);\
if(!strcpy( dest , src )){\
panic_message("Could not copy " #src " to " #dest ".\n");\
}\
} while(0);

#define safefree(pointer) do{ if( pointer ) free( pointer );\
pointer = NULL;\
} while(0);

void panic(void);
void panic_message(char *name);

#endif /* _PANIC_H */
