#include <services.h>
#include <stdlib.h>
#include <string.h>

#define STACK_PTR_SIZE 16

typedef struct {
    void *vals[STACK_PTR_SIZE];
    int top;
} ptrstack_t;

static ptrstack_t savedPtrs = {{}, 0};

char *getMetaValue(metanode_t **metadata, char *key){
    metanode_t *node;

    node = *metadata;
    if(EMPTY(node))
        return NULL;
    else do{
        if(!strcasecmp(key, node->name)){
            return node->value;
        }
    }while(ITER(node));
    return NULL;
}

metanode_t *setMetaValue(metanode_t **metadata, char *key, char *value){
    metanode_t *node, *prev, *newnode;

    prev = node = *metadata;
    if(EMPTY(node)){
        safemalloc(newnode, metanode_t);
        safenmalloc(newnode->name, char, strlen(key)+1);
        strcpy(newnode->name, key);
        safenmalloc(newnode->value, char, strlen(value)+1);
        strcpy(newnode->value, value);
        newnode->next = NULL;
        *metadata = newnode;
        return newnode;
    }else do{
        if(!strcasecmp(key, node->name)){
            safefree(node->value);
            safenmalloc(node->value, char, strlen(value)+1);
            strcpy(node->value, value);
            return node;
        }
        prev = node;
    }while(ITER(node));
    safemalloc(newnode, metanode_t);
    safenmalloc(newnode->name, char, strlen(key)+1);
    strcpy(newnode->name, key);
    safenmalloc(newnode->value, char, strlen(value)+1);
    strcpy(newnode->value, value);
    newnode->next = NULL;
    prev->next = newnode;
    return newnode;
}

void delMetaValue(metanode_t **metadata, char *key){
    metanode_t *node, *prev;
    prev = node = *metadata;
    if(EMPTY(node))
        return;
    if(!strcasecmp(key, node->name)){
        safefree(node->value);
        *metadata = node->next;
        safefree(node);
        return;
    }
    while(ITER(node)){
        if(!strcasecmp(key, node->name)){
            safefree(node->name);
            safefree(node->value);
            prev->next = node->next;
            safefree(node);
            return;
        }
        prev = node;
    }
}

void clearMetadata(metanode_t **metadata){
    metanode_t *node, *prev;
    node = *metadata;
    if(EMPTY(node))
        return;
    while(node){
        prev = node;
        free(prev->name);
        free(prev->value);
        ITER(node);
        free(prev);
    }
    *metadata = NULL;
}

int pushPtr(void *ptr){
    if(savedPtrs.top < STACK_PTR_SIZE){
        savedPtrs.vals[savedPtrs.top++] = ptr;
    }else{
        return -1;
    }
    return STACK_PTR_SIZE-savedPtrs.top;
}
void *popPtr(){
    if(!savedPtrs.top)
        return NULL;
    return savedPtrs.vals[--savedPtrs.top];
}
