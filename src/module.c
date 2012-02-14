#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>

#include <module.h>
#include <acuity.h>
#include <actypes.h>

modnode *modlist = NULL;

void *loadModule(const char *modname){
    char *fname;
    void *modhandle;
    modnode *node;

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
    return modhandle;
}

void unloadModules(){
    modnode *node, *prev;
    node = modlist;
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
    modlist = NULL;
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
