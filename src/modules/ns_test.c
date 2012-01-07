
#include <services.h>
#include <stdio.h>
#include "nickserv.h"

void testCmd2(char *uid, char *msg){
    char buff[128];
    sprintf(buff,":%s NOTICE %s :Test2 succeeded.\r\n",nickserv->uid,uid);
    send_raw_line(buff);
    aclog(LOG_DEBUG,"Test2 message sent: %s\n",msg);
}

void INIT_MOD(){
    registerNickServCommand("testx",testCmd2);
}
