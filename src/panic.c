/* THE PANIC SYSTEM
concept: memory is gone/something crashed badly, so, whatever,
we should abort softly.
Only use raw commands, we can't really buffer stuff.
*/
#include <services.h>
#include <stdlib.h>

char panic_buffer_reserves[] = "";/* reserve more space as needed later */

void panic(){
    aclog(LOG_ERROR, "We've come across a horrible error that has caused us "
        "to cease all functionality. That is all. [TERMINATED]\n");
    unloadModules();
    exit(1);
}

void panic_message(char *message){
    aclog(LOG_ERROR, message);
    panic();
}
