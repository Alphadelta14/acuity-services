#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>

#include <module.h>
#include <acuity.h>

modnode *modlist = NULL;

void *loadModule(const char *modname){
    char *fname;
    void *modhandle;
    modnode *node, *prev;

    aclog(LOG_MODINFO,"Loading module: %s",modname);
    fname = (char*)malloc(sizeof(char)*(strlen(modname)+13));//"modules/%s.so"
    if(!fname){
        aclog(LOG_ERROR,"Not enough memory to allocate `fname` for `loadModule`.\n");
        return NULL;
    }
    strcpy(fname,"modules/");
    strcat(fname,modname);
    strcat(fname,".so");
    modhandle = dlopen(fname,RTLD_NOW|RTLD_GLOBAL);
    free(fname);
    if(!modhandle){
        aclog(LOG_MODINFO,"  [FAIL]\n");
        aclog(LOG_ERROR,"Could not open '%s' module: %s\n",modname,dlerror());
        return NULL;
    }
    *(void **) (&initModule) = dlsym(modhandle, "INIT_MOD");

    node = (modnode*)malloc(sizeof(modnode));
    if(!node){
        aclog(LOG_ERROR,"Could not saved loaded module %s. Could not allocate memory.\n",modname);
        return modhandle;
    }
    node->name = (char*)malloc(sizeof(char)*(strlen(modname)+1));
    if(!node->name){
        aclog(LOG_ERROR,"Could not save module node name. Could not allocate memory.\n");
        return modhandle;
    }
    strcpy(node->name,modname);
    node->next = NULL;
    if(!modlist)
        modlist = node;
    else{
        prev = modlist;
        while(prev->next) prev = prev->next;
        prev->next = node;
    }
    aclog(LOG_MODINFO,"  [OK]\n");
    if(initModule)
        initModule();
    return modhandle;
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
    aclog(LOG_MODINFO,"%d depends\n",count);
    for(i=0;i<count;i++){
        modname = va_arg(args,char*);
        aclog(LOG_MODINFO,"Checking for %s\n",modname);
        if(!isModuleLoaded(modname)){
            aclog(LOG_MODINFO,"Loading depended module: %s\t\n",modname);
            loadModule(modname);
        }
    }
    va_end(args);
}
