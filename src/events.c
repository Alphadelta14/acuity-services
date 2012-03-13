#include <services.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

static namedeventnode_t *namedeventlist = NULL;
static timeevent_t *timeeventlist = NULL;

static void addEventToList(eventnode_t **eventlist, void (*callback)(line *L)){
    eventnode_t *N;

    safemalloc(N, eventnode_t);
    N->callback = callback;
    N->next = *eventlist;
    *eventlist = N;
}

static void delEventFromList(eventnode_t **eventlist, void (*callback)(line *L)){
    eventnode_t *node, *prev;

    prev = node = *eventlist;
    if(node->callback==callback){
        *eventlist = node->next;
        safefree(node);
        return;
    }
    do{
        if(node->callback==callback){
            prev->next = node->next;
            safefree(node);
            return;
        }
        prev = node;
    }while(ITER(node));
}

void hook_named_event(char *name, void (*callback)(line *L)){
    namedeventnode_t *namednode;

    namednode = namedeventlist;
    if(namednode)
        do{
            if(!strcasecmp(namednode->name, name)){
                addEventToList(&namednode->eventlist, callback);
                return;
            }
        }while(ITER(namednode));
    safemalloc(namednode, namedeventnode_t);
    safenmalloc(namednode->name, char, strlen(name)+1);
    strcpy(namednode->name, name);
    namednode->eventlist = NULL;
    namednode->next = namedeventlist;
    namedeventlist = namednode;
    addEventToList(&namednode->eventlist, callback);
}

void unhook_named_event(char *name, void (*callback)(line *L)){
    namedeventnode_t *namednode, *prevnode;

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
    do{
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
    }while(ITER(namednode));
}

void fireEventList(eventnode_t *eventlist, line *L){
    eventnode_t *N;

    N = eventlist;
    if(EMPTY(N))
        return;
    do{
        if(N->callback)
            N->callback(L);
    }while(ITER(N));
}

void fire_named_event(char *name, line *L){
    namedeventnode_t *namednode;

    namednode = namedeventlist;
    if(EMPTY(namednode))
        return;
    do{
        if(!strcasecmp(namednode->name, name)){
            fireEventList(namednode->eventlist, L);
            return;
        }
    }while(ITER(namednode));
}

void addTimerEvent(void(*callback)(int argc, char **argv), time_t expires, int argc, ...){
    timeevent_t *T, *S;
    char **argv, *arg;
    va_list ap;
    int i;

    T = timeeventlist;
    if(T){
        S = T;
        while(T->time < expires){
            S = T;
            ITER(T);
            if(!T)
                break;
        }
    } else {
        S = NULL;
    }
    safemalloc(T, timeevent_t);
    va_start(ap, argc);
    safenmalloc(argv, char*, argc);
    for(i = 0; i < argc; i++){
        /* TODO: Sanity - is there a legitimate next argument? */
        arg = va_arg(ap, char*);
        safenmalloc(argv[i], char,strlen(arg)+1);
        strcpy(argv[i], arg);
    }
    T->callback = callback;
    T->argc = argc;
    T->argv = argv;
    T->time = expires;
    if(S){
        T->next = S->next;
        S->next = T;
    } else {
        T->next = NULL;
        timeeventlist = T;
    }
}

void onTimer(){
    int i;
    time_t now;
    timeevent_t *T, *prev;

    if(EMPTY(timeeventlist))
        return;
    now = time(NULL);
    T = timeeventlist;
    while((T)&&(T->time <= now)){
        T->callback(T->argc, T->argv);
        for(i = 0; i < T->argc; i++){
            safefree(T->argv[i]);
        }
        safefree(T->argv);
        prev = T;
        ITER(T);
        safefree(prev);
    }
    timeeventlist = T;
}
