#include <actypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <events.h>
#include <acuity.h>

eventnode *eventlist[NUM_EVENTS] = {NULL};
expirylist timerList = {NULL};

void hook_event(int event, void (*callback)(line *L)){
    eventnode *N;
    if(event >= NUM_EVENTS)
        return;
    N = eventlist[event];
    if(!N){
        safemallocvoid(N, eventnode);
        N->next = NULL;
        N->callback = callback;
        eventlist[event] = N;
        return;
    }
    while(N->next)
        N = N->next;
    safemallocvoid(N->next, eventnode);
    if(!N->next){
        aclog(LOG_ERROR,"hook_event failed: unable to allocate memory\n");
        return;
    }
    N = N->next;
    N->callback = callback;
    N->next = NULL;
}

void unhook_event(int event, void (*callback)(line *L)){
    eventnode *node, *prev;
    if(event >= NUM_EVENTS)
        return;
    prev = node = eventlist[event];
    if(node->callback==callback){
        eventlist[event] = node->next;
        safefree(node);
        return;
    }
    while(node){
        if(node->callback==callback){
            prev->next = node->next;
            safefree(node);
            return;
        }
        prev = node;
        node = node->next;
    }

}

void fire_event(int event, line *L){
    eventnode *N;

    if(event >= NUM_EVENTS)
        return;
    N = eventlist[event];
    while(N){
        if(N->callback)
            N->callback(L);
        N = N->next;
    }
}

void addTimerEvent(void(*callback)(int argc, char **argv), time_t expires, int argc, ...){
    timer *t, *s;
    char **argv, *arg;
    va_list ap;
    int i;

    t = timerList.first;
    if(t){
        s = t;
        while(t->time < expires){
            s = t;
            t = t->next;
            if(!t)
                break;
        }
    } else {
        s = NULL;
    }
    safemallocvoid(t,timer);
    va_start(ap, argc);
    safenmallocvoid(argv,char*,sizeof(char*)*argc);
    for(i=0; i<argc; i++){
        /* TODO: Sanity - is there a legitimate next argument? */
        arg = va_arg(ap, char*);
        safenmallocvoid(argv[i],char,sizeof(char)*(strlen(arg)+1));
        strcpy(argv[i], arg);
    }
    t->callback = callback;
    t->argc = argc;
    t->argv = argv;
    t->time = expires;
    if(s){
        t->next = s->next;
        s->next = t;
    } else {
        t->next = NULL;
        timerList.first = t;
    }
}

void onTimer(){
    int i;
    time_t now;
    timer *t, *prev;
    //printf("checking timers\n");
    if(!timerList.first)
        return;
    now = time(NULL);
    t = timerList.first;
    while((t)&&(t->time <= now)){
        t->callback(t->argc, t->argv);
        for(i=0;i<t->argc;i++){
            free(t->argv[i]);
        }
        free(t->argv);
        prev = t;
        t = t->next;
        free(prev);
    }
    timerList.first = t;
}
