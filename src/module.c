#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>

#include <module.h>
#include <acuity.h>
#include <actypes.h>

modnode *modlist = NULL;
int MOD_STATE = MOD_IDLE;

void *loadModule(const char *modname){
    char *fname;
    void *modhandle;
    modnode *node;

    MOD_STATE = MOD_LOAD;
    aclog(LOG_DEBUG,"Loading module: %s",modname);
    safenmalloc(fname,char,sizeof(char)*(strlen(modname)+13),NULL);/*"modules/%s.so"*/
    strcpy(fname,"modules/");
    strcat(fname,modname);
    strcat(fname,".so");
    modhandle = dlopen(fname,RTLD_NOW|RTLD_GLOBAL);
    free(fname);
    if(!modhandle){
        aclog(LOG_DEBUG,"  [FAIL]\n");
        aclog(LOG_ERROR,"Could not open '%s' module: %s\n",modname,dlerror());
        return NULL;
    }
    *(void **) (&initModule) = dlsym(modhandle, "INIT_MOD");
    safemalloc(node,modnode,NULL);
    node->handle = modhandle;
    safenmalloc(node->name,char,sizeof(char)*(strlen(modname)+1),NULL);
    strcpy(node->name,modname);
    node->next = modlist;
    modlist = node;
    aclog(LOG_DEBUG,"  [OK]\n");
    if(initModule)
        initModule();
    MOD_STATE = MOD_IDLE;
    return modhandle;
}

void unloadModule(char *name){
    modnode *node, *prev;
    node = modlist;
    MOD_STATE = MOD_UNLOAD;
    while(node){
        if(!strcasecmp(node->name, name)){
            aclog(LOG_DEBUG,"Unloading module: %s", node->name);
            *(void **) (&termModule) = dlsym(node->handle, "TERM_MOD");
            if(termModule)
                termModule();
            free(node->name);
            prev->next = node->next;
            free(node);
            aclog(LOG_DEBUG,"\t[OK]\n");
            return;
        }
        prev = node;
        node = node->next;
    }
}

void unloadModules(){
    modnode *node, *prev;
    node = modlist;
    MOD_STATE = MOD_UNLOAD;
    while(node){
        aclog(LOG_DEBUG,"Unloading module: %s", node->name);
        *(void **) (&termModule) = dlsym(node->handle, "TERM_MOD");
        if(termModule)
            termModule();
        prev = node;
        node = node->next;
        free(prev->name);
        free(prev);
        aclog(LOG_DEBUG,"\t[OK]\n");
    }
    MOD_STATE = MOD_IDLE;
    modlist = NULL;
}

void reloadModule(char *modname){
    unloadModule(modname);
    loadModule(modname);
}

char isModuleLoaded(char *modname){
    modnode *node;
    node = modlist;
    if(!node)
        return 0;
    while(node){
        if(!strcasecmp(node->name,modname)){
            return 1;
        }
        node = node->next;
    }
    return 0;
}

void loadDependencies(int count, ...){
    int i;
    va_list args;
    char *modname;
    va_start(args,count);
    aclog(LOG_DEBUG,"%d depends\n",count);
    for(i=0;i<count;i++){
        modname = va_arg(args,char*);
        aclog(LOG_DEBUG,"Checking for %s\n",modname);
        if(!isModuleLoaded(modname)){
            aclog(LOG_DEBUG,"Loading depended module: %s\t\n",modname);
            loadModule(modname);
        }
    }
    va_end(args);
}
