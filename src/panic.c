/* THE PANIC SYSTEM
concept: memory is gone/something crashed badly, so, whatever, we should abort softly.
Only use raw commands, we can't really buffer stuff.
*/
#include <services.h>
#include <stdio.h>
#include <stdlib.h>

char panic_buffer_reserves[] = "";

void panic(){
    
    printf("We've come across a horrible error that has caused us to cease all functionality. That is all. [TERMINATED]\n");
    exit(1);
}
