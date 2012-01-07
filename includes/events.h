#ifndef _EVENTS_H
#define _EVENTS_H
#include <actypes.h>
#include <stdarg.h>

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

typedef struct _eventnode {
    void (*callback)(line *L);
    struct _eventnode *next;
} eventnode;

typedef struct _timer {
    void(*callback)(int argc, char **argv);
    int argc;
    char **argv;
    time_t time;
    struct _timer *next;
} timer;

typedef struct _expirylist {
    timer *first;
} expirylist;

extern expirylist timerList;
extern eventnode eventlist[];

void fire_event(int event, line *L);
void hook_event(int event, void (*callback)(line *L));
void addTimerEvent(void(*callback)(int argc, char **argv), time_t expires, int argc, ...);
void onTimer(void);

#endif /* _EVENTS_H */
