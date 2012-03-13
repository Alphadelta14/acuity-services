#ifndef _EVENTS_H
#define _EVENTS_H

#include "services.h"

typedef struct line_s line;

void fire_event(char *name, line *L);
void hook_event(char *name, void (*callback)(line *L));
void unhook_event(char *name, void (*callback)(line *L));
void add_timer_event(void(*callback)(int argc, char **argv), time_t expires, int argc, ...);
void fire_timer_event(void);

#endif /* _EVENTS_H */
