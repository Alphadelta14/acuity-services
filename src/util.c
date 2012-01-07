#include <actypes.h>
#include <acuity.h>
#include <stdlib.h>
#include <string.h>

char *getMetaValue(metanode *metadata, char *key){
    metanode *node;
    node = metadata;
    while(node){
        if(!strcasecmp(key,node->name)){
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

metanode *setMetaValue(metanode *metadata, char *key, char *value){
    metanode *node, *newnode;
    if(!metadata){
        safemalloc(newnode,metanode,NULL);
        safenmalloc(newnode->name,char,strlen(key)+1,NULL);
        strcpy(newnode->name,key);
        safenmalloc(newnode->value,char,strlen(value)+1,NULL);
        strcpy(newnode->value,value);
        newnode->next = NULL;
        metadata = newnode;
        return metadata;
    }
    node = metadata;
    while(node->next){
        if(!strcasecmp(key,node->name)){
            free(node->value);
            safenmalloc(node->value,char,strlen(value)+1,NULL);
            strcpy(node->value,value);
            return node;
        }
        node = node->next;
    }
    safemalloc(newnode,metanode,NULL);
    safenmalloc(newnode->name,char,strlen(key)+1,NULL);
    strcpy(newnode->name,key);
    safenmalloc(newnode->value,char,strlen(value)+1,NULL);
    strcpy(newnode->value,value);
    newnode->next = NULL;
    node->next = newnode;
    return newnode;
}
