#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dlfcn.h>

#include <acuity.h>
#include <events.h>
#include <config.h>
#include <ircd.h>
#include <network.h>
#include <module.h>

const char __version__[] = "Acuity-alpha1";

/* local to just this file */
void sighup();
void sigquit();
int acuity_start();
int acuity_stop();
int acuity_rehash();

void errprintf(int level, char *fmt, ...){
    va_list args;
    va_start(args,fmt);
    vfprintf(stderr,fmt,args);
    va_end(args);
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
    printf("got SIGQUIT\n");
    exit(1);
}

int acuity_start(){
    //void *handle;
    FILE *fpid;
    //int i;
    //eventnode *en;

    fpid = fopen("acuity.pid","w");
    fprintf(fpid,"%d",getpid());
    fclose(fpid);
    signal(SIGHUP,sighup);
    signal(SIGQUIT,sigquit);
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
    loadModule("network");
    loadModule("inspircd20");
    loadModule("nickserv");
    //loadModule("ns_test");
    aclog(5,"Connecting\n");
    if(!connectIRC(getConfigValue("HubHost"),getConfigValue("HubPort"))){
        return 1;
    }
    eventloopIRC();
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
