#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <services.h>

const char __version__[] = "Acuity-alpha2";
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

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s (start|stop|rehash)\n", argv[0]);
        return 0;
    }
    if(!strcmp(argv[1], "start")){
        return acuity_start();
    }else if(!strcmp(argv[1], "stop")){
        return acuity_stop();
    }else if(!strcmp(argv[1], "rehash")){
        return acuity_rehash();
    }else{
        printf("Usage: %s (start|stop|rehash)\n", argv[0]);
    }
    return 0;
}


int acuity_start(){
    FILE *fpid;

    fpid = fopen("acuity.pid", "w");
    fprintf(fpid, "%d", getpid());
    fclose(fpid);
    log_init();
    signal(SIGHUP, sighup);
    signal(SIGQUIT, sigquit);
    signal(SIGINT, sigquit);
    /* Now, our uptime may start! */
    starttime = time(NULL);
    return 0;
}

int acuity_stop(){
    FILE *fpid;
    int pid, fail;

    fpid = fopen("acuity.pid", "r");
    if(!fpid){
        printf("PID file does not exist or is not readable.\n");
        return 1;
    }
    fscanf(fpid, "%d", &pid);
    fclose(fpid);
    fail = kill(pid, SIGQUIT);
    if(!fail){
        printf("Successfully killed %d.\n",pid);
    }else{
        printf("Failed to kill %d.\n",pid);
    }
    return fail;
}

int acuity_rehash(){
    FILE *fpid;
    int pid, fail;

    fpid = fopen("acuity.pid", "r");
    if(!fpid){
        printf("PID file does not exist or is not readable.\n");
        return 1;
    }
    fscanf(fpid, "%d", &pid);
    fclose(fpid);
    fail = kill(pid, SIGHUP);
    if(!fail){
        printf("Sent rehash signal to %d.\n",pid);
    }else{
        printf("Failed to rehash %d.\n",pid);
    }
    return fail;
}


void sighup(){
    printf("got SIGHUP\n");
    exit(0);
}

void sigquit(){
    exit(0);
}
