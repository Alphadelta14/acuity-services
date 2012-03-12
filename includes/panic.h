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

#define safefree(pointer) do{ if( pointer ) free( pointer );\
pointer = NULL;\
} while(0);

void panic(void);
void panic_message(char *name);

#endif /* _PANIC_H */
