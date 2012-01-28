#include <actypes.h>
#include <acuity.h>
#include <stdlib.h>
#include <string.h>

char *getMetaValue(metanode **metadata, char *key){
    metanode *node;
    node = *metadata;
    while(node){
        if(!strcasecmp(key,node->name)){
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

metanode *setMetaValue(metanode **metadata, char *key, char *value){
    metanode *node, *prev, *newnode;
    if(!*metadata){
        safemalloc(newnode,metanode,NULL);
        safenmalloc(newnode->name,char,strlen(key)+1,NULL);
        strcpy(newnode->name,key);
        safenmalloc(newnode->value,char,strlen(value)+1,NULL);
        strcpy(newnode->value,value);
        newnode->next = NULL;
        *metadata = newnode;
        return newnode;
    }
    prev = node = *metadata;
    while(node){
        if(!strcasecmp(key,node->name)){
            free(node->value);
            safenmalloc(node->value,char,strlen(value)+1,NULL);
            strcpy(node->value,value);
            return node;
        }
        prev = node;
        node = node->next;
    }
    safemalloc(newnode,metanode,NULL);
    safenmalloc(newnode->name,char,strlen(key)+1,NULL);
    strcpy(newnode->name,key);
    safenmalloc(newnode->value,char,strlen(value)+1,NULL);
    strcpy(newnode->value,value);
    newnode->next = NULL;
    prev->next = newnode;
    return newnode;
}

void delMetaValue(metanode **metadata, char *key){
    metanode *node, *prev;
    prev = node = *metadata;
    if(!*metadata)
        return;
    if(!strcasecmp(key, node->name)){
        free(node->value);
        *metadata = node->next;
        free(node);
        return;
    }
    while(node){
        if(!strcasecmp(key, node->name)){
            free(node->name);
            free(node->value);
            prev->next = node->next;
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void clearMetadata(metanode **metadata){
    metanode *node, *prev;
    node = *metadata;
    while(node){
        prev = node;
        free(prev->name);
        free(prev->value);
        node = node->next;
        free(prev);
    }
    *metadata = NULL;
}
