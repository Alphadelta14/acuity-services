#include <actypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <events.h>
#include <acuity.h>

eventnode *eventlist[NUM_EVENTS] = {NULL};
namedeventnode *namedeventlist = NULL;
expirylist timerList = {NULL};

static void addEventToList(eventnode **eventlist, void (*callback)(line *L)){
    eventnode *N;
    N = *eventlist;
    if(!N){
        safemallocvoid(N, eventnode);
        N->next = NULL;
        N->callback = callback;
        *eventlist = N;
        return;
    }
    while(N->next)
        N = N->next;
    safemallocvoid(N->next, eventnode);
    N = N->next;
    N->callback = callback;
    N->next = NULL;
}

static void delEventFromList(eventnode **eventlist, void (*callback)(line *L)){
    eventnode *node, *prev;
    prev = node = *eventlist;
    if(node->callback==callback){
        *eventlist = node->next;
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

void hook_event(int event, void (*callback)(line *L)){
    if(event<NUM_EVENTS)
        addEventToList(&eventlist[event], callback);
}

void unhook_event(int event, void (*callback)(line *L)){
    if(event<NUM_EVENTS)
        delEventFromList(&eventlist[event], callback);
}

void hook_named_event(char *name, void (*callback)(line *L)){
    namedeventnode *namednode;
    namednode = namedeventlist;
    while(namednode){
        if(!strcasecmp(namednode->name, name)){
            addEventToList(&namednode->eventlist, callback);
            return;
        }
        namednode = namednode->next;
    }
    safemallocvoid(namednode, namedeventnode);
    safenmallocvoid(namednode->name, char, strlen(name)+1);
    strcpy(namednode->name, name);
    namednode->eventlist = NULL;
    namednode->next = namedeventlist;
    namedeventlist = namednode;
    addEventToList(&namednode->eventlist, callback);
}

void unhook_named_event(char *name, void (*callback)(line *L)){
    namedeventnode *namednode, *prevnode;
    prevnode = namednode = namedeventlist;
    if(!strcasecmp(namednode->name, name)){
        delEventFromList(&namednode->eventlist, callback);
        if(!namednode->eventlist){
            namedeventlist = namednode->next;
            safefree(namednode->name);
            safefree(namednode);
        }
        return;
    }
    while(namednode){
        if(!strcasecmp(namednode->name, name)){
            delEventFromList(&namednode->eventlist, callback);
            if(!namednode->eventlist){
                prevnode->next = namednode->next;
                safefree(namednode->name);
                safefree(namednode);
            }
            return;
        }
        prevnode = namednode;
        namednode = namednode->next;
    }
}

void fireEventList(eventnode *eventlist, line *L){
    eventnode *N;
    N = eventlist;
    while(N){
        if(N->callback)
            N->callback(L);
        N = N->next;
    }
}

void fire_event(int event, line *L){
    if(event >= NUM_EVENTS)
        return;
    fireEventList(eventlist[event], L);
}

void fire_named_event(char *name, line *L){
    namedeventnode *namednode;
    namednode = namedeventlist;
    while(namednode){
        if(!strcasecmp(namednode->name, name)){
            fireEventList(namednode->eventlist, L);
            return;
        }
        namednode = namednode->next;
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
