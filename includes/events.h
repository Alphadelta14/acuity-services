#ifndef _EVENTS_H
#define _EVENTS_H

#include "services.h"

typedef struct line_s line_t;

typedef struct eventpacket_s {
    line_t *line;
    int argc;
    char **argv;
} eventpacket_t;

void fire_event(char *name, line_t *line, int argc, ...);
void hook_event(char *name, void (*callback)(eventpacket_t event));
void unhook_event(char *name, void (*callback)(eventpacket_t event));
void add_timer_event(void (*callback)(eventpacket_t event), time_t expires, int argc, ...);
void fire_timer_event(void);

#endif /* _EVENTS_H */
