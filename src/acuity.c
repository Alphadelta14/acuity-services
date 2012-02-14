#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>

#include <services.h>

const char __version__[] = "Acuity-alpha1";
/* Used for uptime checks:
 * - WHOIS idle time
 * - Later on maybe, STATS u or similar
 * - Later on maybe, biggest uptime ever in db? */
time_t starttime;

/* local to just this file */
void sighup();
void sigquit();
int acuity_start();
int acuity_stop();
int acuity_rehash();

void errprintf(int flags, ...){
    char *fmt;
    /*user *src;*/
    va_list args;
    va_start(args, flags);
    if(flags&LOGFLAG_SRC){
        va_arg(args, user*);/* src */
    }
    fmt = va_arg(args, char*);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void _closeIRC(char *quit){
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc==1){
        printf("Usage: %s (start|stop|rehash)\n",argv[0]);
        return 0;
    }
    if(!strcmp(argv[1],"start")){
        return acuity_start();
    } else if(!strcmp(argv[1],"stop")){
        return acuity_stop();
    } else if(!strcmp(argv[1],"rehash")){
        return acuity_rehash();
    } else {
        printf("Usage: %s (start|stop|rehash)\n",argv[0]);
    }
    return 0;
}

void sighup(){
    printf("got SIGHUP\n");
    exit(1);
}
void sigquit(){
    aclog(LOG_ERROR, "Acuity received QUIT signal. Shutting down.\n");
    closeIRC("Acuity received QUIT signal. Shutting down.");
    exit(0);
}

int acuity_start(){
    //void *handle;
    FILE *fpid;
    //int i;
    //eventnode *en;

    fpid = fopen("acuity.pid", "w");
    fprintf(fpid, "%d", getpid());
    fclose(fpid);
    closeIRC = &_closeIRC;
    signal(SIGHUP, sighup);
    signal(SIGQUIT, sigquit);
    signal(SIGINT, sigquit);
#ifndef DEVEL
    signal(SIGSEGV, panic);/* only hook on released versions */
#endif /* DEVEL */
    aclog = &errprintf;
    /*en = eventlist;
    for(i=0; i<NUM_EVENTS; i++){
        en->
    }*/

    /* if(!(handle = dlopen("modules/config.so", RTLD_NOW))){
        printf("%s\n",dlerror());
        return 1;
    }
    *(void **) (&conf) = dlsym(handle, "DEFAULTCONF");
    conf();
    *(void **) (&conf) = dlsym(handle, "CONFIG");
    conf();
    *(void **) (&conf) = dlsym(handle, "INFO");
    conf();
    //server = *(serverconn*)dlsym(handle, "server");
    printf("%d\n",server.port);

    dlclose(handle); */
    loadConfig();
    open_database();
    loadPermissions();
    loadModule("network");
    loadModule("inspircd20");
    loadModule("nickserv");
    loadModule("operserv");
    //loadModule("ns_test");
    aclog(5,"Connecting\n");
    if(!connectIRC(getConfigValue("HubHost"),getConfigValue("HubPort"))){
        return 1;
    }
    /* Now, our uptime may start! */
    starttime = time(NULL);
    eventloopIRC();
    close_database();
    return 0;
}

int acuity_stop(){
    FILE *fpid;
    int pid, fail;

    fpid = fopen("acuity.pid","r");
    if(!fpid){
        printf("PID file does not exist or is not readable.\n");
        return 1;
    }
    fscanf(fpid, "%d", &pid);
    fail = kill(pid, SIGQUIT);
    if(!fail){
        printf("Successfully killed %d.\n",pid);
    } else {
        printf("Failed to kill %d.\n",pid);
    }
    return fail;
}

int acuity_rehash(){
    FILE *fpid;
    int pid, fail;

    fpid = fopen("acuity.pid","r");
    if(!fpid){
        printf("PID file does not exist or is not readable.\n");
        return 1;
    }
    fscanf(fpid, "%d", &pid);
    fail = kill(pid, SIGHUP);
    if(!fail){
        printf("Sent rehash signal to %d.\n",pid);
    } else {
        printf("Failed to rehash %d.\n",pid);
    }
    return fail;
}
