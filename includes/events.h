#ifndef _EVENTS_H
#define _EVENTS_H

#include "services.h"

/* EVENT_ERR/EVENT_NULL 0 <- reserve 0 for now */
#define EVENT_LINK 1
#define EVENT_ENDLINK 2
#define EVENT_PING 3
#define EVENT_CONNECT 4
#define EVENT_QUIT 5
#define EVENT_NICK 6
#define EVENT_JOIN 7
#define EVENT_PART 8
#define EVENT_KICK 9
#define EVENT_MODE 10
#define EVENT_MESSAGE 11
#define EVENT_TOPIC 12

#define NUM_EVENTS 13

typedef struct line_s line;

typedef struct eventnode_s {
    void (*callback)(line *L);
    struct eventnode_s *next;
} eventnode_t;

typedef struct namedeventnode_s {
    char *name;
    eventnode_t *eventlist;
    struct namedeventnode_s *next;
} namedeventnode_t;

typedef struct timeevent_s {
    void(*callback)(int argc, char **argv);
    int argc;
    char **argv;
    time_t time;
    struct timeevent_s *next;
} timeevent_t;

typedef struct expirylist_s {
    timeevent_t *first;
} expirylist_t;


void fire_event(int event, line *L);
void fire_named_event(char *name, line *L);
void hook_event(int event, void (*callback)(line *L));
void hook_named_event(char *name, void (*callback)(line *L));
void unhook_event(int event, void (*callback)(line *L));
void unhook_named_event(char *name, void (*callback)(line *L));
void addTimerEvent(void(*callback)(int argc, char **argv), time_t expires, int argc, ...);
void onTimer(void);

#endif /* _EVENTS_H */
