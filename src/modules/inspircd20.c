#include <actypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#include <acuity.h>
#include <ircd.h>
#include <services.h>
#include <config.h>
#include <events.h>
#include <network.h>
#include <encrypt.h>


void send_raw_insp(const char *buff){
    write(irc_socket, buff, strlen(buff));
    //printf("r:%s",buff);
}

char cuid[] = "000AAAAA@";
/* first generated is XXXAAAAAA */

/*
:644AAAAAA PRIVMSG #lobby :hi
*/
line parseLineInsp2(char *data){
    line L;
    char *c;
    int paramCount = 0;

    L.command = NULL;
    L.params = NULL;
    L.paramCount = 0;
    L.text = NULL;
    if(!data){
        return L;
    }
    if(data[0] == ':'){
        L.id = data+1;
    } else {
        L.id = NULL;
        L.command = data;
        while((data[0] != ' ')&&(data[0] != '\0')) data++;
        if(data[0] == '\0'){
            return L;
        }
        data[0] = '\0';
        data++;
        c = data;
        while((c[0] != '\0')&&(c[1] != ':')){
            if(c[0] == ' '){
                paramCount++;
            }
            c++;
        }
        paramCount++;/* for the \0 terminated param */
        L.paramCount = paramCount;
        paramCount = 0;
        L.params = (char**)malloc(sizeof(char*)*L.paramCount);
        L.params[paramCount++] = data;
        while((data[0] != '\0')&&(data[1] != ':')){
            if(data[0] == ' '){
                L.params[paramCount++] = data+1;
                data[0] = '\0';
            }
            data++;
        }
        if(data[1] == ':'){
            data[0] = '\0';
            L.text = data+2;
        }
        return L;
    }
    while(data[0] != ' ') data++;
    data[0] = '\0';

    L.command = data+1;
    while(data[0] != ' ') data++;
    data[0] = '\0';
    data++;

    c = data;
    while(c[1] != ':'){
        if(c[0] == ' '){
            paramCount++;
        } else if(c[0] == '\0'){
            break;
        }
        c++;
    }
    L.paramCount = paramCount;
    if(L.paramCount){
        paramCount = 0;
        L.params = (char**)malloc(sizeof(char*)*L.paramCount);
        L.params[paramCount++] = data;
        while(data[1] != ':'){
            if(data[0] == ' '){
                L.params[paramCount++] = data+1;
                data[0] = '\0';
            } else if(data[0] == '\0'){
                break;
            }
            data++;
        }
    }

    if(data[1] == ':')
        L.text = data+2;
    return L;
}
line parseLineInsp(char *data){
    line L;
    char *dataptr;
    int paramCount = 0;

    L.id = NULL;
    L.command = NULL;
    L.params = NULL;
    L.paramCount = 0;
    L.text = NULL;

    if(!data||!data[0]){
        return L;
    }
    if(data[0] == ':'){
        L.id = data+1;
        while(data[0]!=' ') data++;
        data[0] = '\0';
        data++;
    }
    L.command = data;
    while(data[0]!=' '){
        if(data[0]=='\0') return L;
        data++;
    }
    data[0] = '\0';
    
    dataptr = ++data;
    while((dataptr[0] != ':')||((dataptr[0] == ':')&&(dataptr[-1] != ' ')&&(dataptr[-1] != '\0'))){
        /* include the last space, so that paramCount increases */
        if(dataptr[0] == ' '){
            paramCount++;
        } else if(dataptr[0] == '\0'){
            paramCount++;
            break;
        }
        dataptr++;
    }
    L.paramCount = paramCount;
    if(L.paramCount){
        paramCount = 0;
        L.params = (char**)malloc(sizeof(char*)*L.paramCount);
        L.params[paramCount++] = data;
        while((data[1] != ':')||((data[1] == ':')&&(data[0] != ' ')&&(data[0] != '\0'))){
            if(data[0] == ' '){
                L.params[paramCount++] = data+1;
                data[0] = '\0';
            } else if(data[0] == '\0'){
                return L;
            }
            data++;
        }
    }
    if(data[0] == ' '){
        data[0] = '\0';
        data++;
    }
    if(data[0] == ':')
        L.text = data+1;
    return L;
    
}

void freeLine(line *L, char isLocal){
    if(L->paramCount)
        free(L->params);
    if(!isLocal)
        free(L);
}

void printLine(line *L){
    int i;
    printf("%s (%s) +%d:\n",L->command, L->id, L->paramCount);
    for(i=0;i<L->paramCount;i++){
        printf("\tparam[%d]: %s\n",i,L->params[i]);
    }
    if(L->text)
        printf("\ttext: %s\n",L->text);
}

char isUser(char *target){
    /* is first character a number? if so, then it's a user and not a channel */
    return ((target[0]<='9')&&(target[0]>='0'));
}

void updateRemoteConf(char *info){
    metanode *node, *next;
    char *key, *value;
    key = strtok(info,"=");
    value = strtok(NULL," ");
    if(!value)
        return;
    next = (metanode*)malloc(sizeof(metanode));
    if(!next){
        aclog(LOG_ERROR,"Failed to create metanode for remoteconf. Could not allocate memory.\n");
        return;
    }
    next->name = (char*)malloc(sizeof(char)*(strlen(key)+1));
    if(!next->name){
        aclog(LOG_ERROR,"Failed to create metanode key for remoteconf. Could not allocate memory.\n");
        return;
    }
    strcpy(next->name, key);
    next->value = (char*)malloc(sizeof(char)*(strlen(value)+1));
    if(!next->value){
        aclog(LOG_ERROR,"Failed to create metanode value for remoteconf. Could not allocate memory.\n");
        return;
    }
    strcpy(next->value, value);
    next->next = NULL;
    
    node = remoteconf;
    if(!node)
        remoteconf = next;
    else{
        while(node->next) node = node->next;
        node->next = next;
    }
    node = next;
    while(key){
        next = (metanode*)malloc(sizeof(metanode));
        if(!next){
            aclog(LOG_ERROR,"Failed to create metanode for remoteconf. Could not allocate memory.\n");
            return;
        }
        next->name = (char*)malloc(sizeof(char)*(strlen(key)+1));
        if(!next->name){
            aclog(LOG_ERROR,"Failed to create metanode key for remoteconf. Could not allocate memory.\n");
            return;
        }
        strcpy(next->name, key);
        next->value = (char*)malloc(sizeof(char)*(strlen(value)+1));
        if(!next->value){
            aclog(LOG_ERROR,"Failed to create metanode value for remoteconf. Could not allocate memory.\n");
            return;
        }
        strcpy(next->value, value);
        next->next = NULL;
        node->next = next;
        node = next;
        key = strtok(NULL,"=");
        value = strtok(NULL," ");
    }
}

char *getRemoteConfValue(char *key){
    metanode *node;
    node = remoteconf;
    while(node){
        if(!strcasecmp(key,node->name)){
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void sendServerLine(){
    char s[512], hmac[65], *challenge, *pass;
    unsigned char tmpsha[32];
    sha256_context ctx;
    int challen, passlen, i;
    /* sha256( (p xor 0x5C) + sha256((p xor 0x36) + m) )
Where 'm' is the challenge string given in the other side's CAPAB, and 'p' is the password you wish to send. Each character of 'p' should be exclusive-or'ed against 0x5C or 0x36, and in the above equation, + indicates string concatenation, not integer addition. */
    if(!(challenge = getRemoteConfValue("CHALLENGE"))){
        sprintf(s,"SERVER %s %s 0 %s :%s\r\n",
            getConfigValue("ServerName"),getConfigValue("HubPass"),getConfigValue("ServerId"),getConfigValue("ServerDesc"));
        send_raw_line(s);
        return;
    }
    pass = getConfigValue("HubPass");
    challen = strlen(challenge);
    passlen = strlen(pass);
    sprintf(s,"%s%s",pass,challenge);
    for(i=0;i<passlen;i++){
        s[i] ^= 0x5c;
        s[i+passlen] ^= 0x36;
    }
    sha256_starts(&ctx);
    sha256_update(&ctx,(unsigned char*)s+passlen,(unsigned long int)passlen+challen);
    sha256_finish(&ctx,tmpsha);
    for(i=0;i<32;i++){
        sprintf(s+passlen+i*2,"%02x",tmpsha[i]);
    }
    sha256_starts(&ctx);
    sha256_update(&ctx,(unsigned char*)s,(unsigned long int)passlen+32);
    sha256_finish(&ctx,tmpsha);
    for(i=0;i<32;i++){
        sprintf(hmac+i*2,"%02x",tmpsha[i]);
    }
    sprintf(s,"SERVER %s HMAC-SHA256:%s 0 %s :%s\r\n",
        getConfigValue("ServerName"),hmac,getConfigValue("ServerId"),getConfigValue("ServerDesc"));
    printf("Sending server line: %s",s);
    send_raw_line(s);
}

void makeChallenge(char *dest, int len){
    int i;
    char tmp;
    FILE *f;
    f = fopen("/dev/urandom","r");
    for(i=0;i<len;i++){
        tmp = fgetc(f)&0x3f;
        dest[i] = tmp+60;
    }
    dest[len] = '\0';
    fclose(f);
}

int connectInsp(char *host, char *port){
//file:///home/david/Dropbox/Public/bot/ircbot.c
    struct addrinfo hints, *res=NULL;
    int err;
    hints.ai_flags = 0;
    hints.ai_family = -1;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    while(1){
        /* Pretty much a for...each...in loop */
        if(hints.ai_family == -1){
            hints.ai_family = AF_INET6;
        } else if(hints.ai_family == AF_INET6){
            hints.ai_family = AF_INET;
        } else {
            break;
        }
        if(res){
            freeaddrinfo(res);
            res = NULL;
        }
        getaddrinfo(host, port, &hints, &res);
        if(!res)
            continue;
        irc_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        err = connect(irc_socket, res->ai_addr, res->ai_addrlen);
        if(!err)
            break;
    }
    if(res)
        freeaddrinfo(res);
    if(err||!irc_socket){
        aclog(2,"Cannot connect to IRCd. Error: %d\n",err);
        return 0;
    }
    return 1;
}


int eventloopInsp(){
    char buff[1024], *buffptr, *c, *tok=NULL, isFrag;
    line l;
    int len, i;

    if(!irc_socket)
        return -1;
    for(i=0;i<1024;i++) buff[i] = 0;
    buffptr = buff;
    //send_raw_line("CAPAB CAPABILITIES :PROTOCOL=1202\r\nCAPAB END\r\nSERVER services.pokesplash.net as2bitz 0 3AZ :PseudoServ\r\n:3AZ BURST\r\n:3AZ VERSION :Something broked\r\n:3AZ ENDBURST\r\n");
    while((len = read(irc_socket, buffptr, 512))){
        *(buffptr+len) = '\0';
        isFrag = buff[strlen(buff)-1];
        isFrag = ((isFrag!='\n')&&(isFrag!='\r'));
        //printf("B: %s\n",buff);
        buffptr = strtok_r(buff,"\r\n",&tok);
        while(1){
            c = strtok_r(NULL,"\r\n",&tok);
            if(isFrag&&(!c)) break;

            //printf("b: %s\n",buffptr);
            l = parseLine(buffptr);
            handleLine(&l);
            freeLine(&l,1);
            
            if(!c) break;
            buffptr = c;
        }
        if(isFrag){
            c = buff;
            do {
                c[0] = buffptr[0];
                buffptr++;
                c++;
            } while(buffptr[0]!='\0');
            buffptr = c;
            continue;
        }
        buffptr = buff;
        onTimer();
    }
    return 0;
}

void handleLineInsp(line *l){
    /* let's give everyone some nice variables to work with */
    char *a;
    char s[512], t[64];
    user *U;

    if(!strcmp(l->command,"PING")){
        sprintf(s,":%s PONG %s %s\r\n",getConfigValue("ServerId"),l->params[1],l->params[0]);
        send_raw_line(s);
        fire_event(EVENT_PING,l);
    } else if(!strcmp(l->command,"CAPAB")){
        /*printLine(l);*/
        if(l->paramCount<1)
            return;
        if(!strcmp(l->params[0],"CAPABILITIES")){
            if(l->text)
                updateRemoteConf(l->text);
/*CAPAB ((null)) +1:
	param[0]: CAPABILITIES
	text: NICKMAX=32 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=256 MAXTOPIC=308 MAXKICK=256 MAXGECOS=129 MAXAWAY=201 IP6SUPPORT=1 PROTOCOL=1202 CHALLENGE=gLKvIN{mvTOq\a}ZU|ve PREFIX=(Yqaohv)!~&@%+ CHANMODES=IXbe,k,FLjl,ACKMNOQRSTcimnprst USERMODES=,,s,ABHIRSWcdhikorwx SVSPART=1*/
        } else if(!strcmp(l->params[0],"END")){
            /*sendServerLine();*/
            a = getConfigValue("ServerId");
            strncpy(cuid,a,3);
            sprintf(s,":%s BURST\r\n:%s VERSION :%s\r\n",a,a,__version__);
            send_raw_line(s);
            fire_event(EVENT_LINK,l);
            sprintf(s,":%s ENDBURST\r\n",a);
            send_raw_line(s);
        }
        if(l->paramCount<2)
            return;
        if(!strcmp(l->params[0],"START")){
            makeChallenge(t,20);
            sprintf(s,"CAPAB START %s\r\nCAPAB CAPABILITIES :PROTOCOL=%s\r\nCAPAB END\r\n",l->params[1],l->params[1]);
            send_raw_line(s);
            sprintf(s,"SERVER %s %s 0 %s :%s\r\n",
                getConfigValue("ServerName"),getConfigValue("HubPass"),getConfigValue("ServerId"),getConfigValue("ServerDesc"));
            send_raw_line(s);
            //raw("CAPAB CAPABILITIES :PROTOCOL=1202\r\nCAPAB END\r\nSERVER services.pokesplash.net as2bitz 0 3AZ :PseudoServ\r\n:3AZ BURST\r\n:3AZ VERSION :Something broked\r\n:3AZ ENDBURST\r\n");
        }
    } else if(!strcmp(l->command,"PRIVMSG")){
        fire_event(EVENT_MESSAGE,l);
/*PRIVMSG (644AAAAAA) +1:
	param[0]: 4CEAAAAAB
	text: hi*/
    } else if(!strcmp(l->command,"BURST")){
/*BURST (644) +1:
	param[0]: 1325188756*/
    } else if(!strcmp(l->command,"ENDBURST")){
        fire_event(EVENT_ENDLINK,l);
/*ENDBURST (644) +0:*/
    } else if(!strcmp(l->command,"VERSION")){
/*VERSION (644) +0:
	text: InspIRCd-2.0 ext.pokesplash.net :*/
    } else if(!strcmp(l->command,"SERVER")){
/*SERVER ((null)) +4:
	param[0]: ext.pokesplash.net
	param[1]: as2bitz
	param[2]: 0
	param[3]: 644
	text: AlphaServ*/
    } else if(!strcmp(l->command,"ADDLINE")){
/*ADDLINE (644) +5:
	param[0]: Q
	param[1]: ChanServ
	param[2]: <Config>
	param[3]: 1325174985
	param[4]: 0
	text: Reserved For Services*/
    } else if(!strcmp(l->command,"FJOIN")){
        printLine(l);
        addChannel(l->params[0],l->params+2);
        if(l->text){
            chanStatusAppend(l->params[0],l->text);
        } else {
            chanStatusAppend(l->params[0],l->params[l->paramCount-1]);
        }
        fire_event(EVENT_JOIN,l);
/*FJOIN (644) +4:
	param[0]: #lobby
	param[1]: 1325186943
	param[2]: +nt
	param[3]: o,644AAAAAA

FJOIN (644) +4:
	param[0]: #lobby
	param[1]: 1325526922
	param[2]: +lnt
	param[3]: 60
	text: o,644AAAAAA ,644AAAAAC*/
    } else if(!strcmp(l->command,"UID")){
        //(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes)
        //printf("%s %s %s %s %s %s %s %s\n",l->params[0],l->params[2],l->params[5],l->params[3],l->params[6],l->params[4],l->text,l->params[8]);
        addUser(l->params[0],l->params[2],l->params[5],l->params[3],l->params[6],l->params[4],l->text,l->params[8]);
        fire_event(EVENT_CONNECT,l);
/*UID (644) +9:
	param[0]: 644AAAAAA
	param[1]: 1325186942
	param[2]: Yellow
	param[3]: 127.0.0.1
	param[4]: as-i9h.1pp.0.127.IP
	param[5]: Amarillo
	param[6]: 127.0.0.1
	param[7]: 1325186947
	param[8]: +ix
	text: Yellow
-- ipv6 --
UID (644) +9:
	param[0]: 644AAAAAB
	param[1]: 1325455521
	param[2]: Yellow
	param[3]: 2001:470:1f06:1057::2
	param[4]: as-k62bl1.ajk7.13gs.0470.2001.IP
	param[5]: Alpha
	param[6]: 2001:470:1f06:1057::2
	param[7]: 1325455526
	param[8]: +ix
	text: Alpha*/
    } else if(!strcmp(l->command,"QUIT")){
        fire_event(EVENT_QUIT,l);
/*QUIT (644AAAAAA) +5:
	param[0]: :Quit:
	param[1]: Connection
	param[2]: reset
	param[3]: by
	param[4]: FEAR*/
    } else if(!strcmp(l->command,"MODE")){
        if(!isUser(l->params[0]))
            U = (user*)getChannel(l->params[0]);
        else
            U = getUser(l->params[0]);
        if(l->paramCount == 2){
            changeMode(&U->modeMinor, &U->modeMajor, l->params[2]);
        } /* TODO: the last parameter */
/*MODE (644) +3:
	param[0]: 644AAAAAA
	param[1]: +s
	param[2]: +aAcCdDfFgGjJkKlLnNoOqQtTvVxX*/
    } else if(!strcmp(l->command,"FMODE")){
        if(!isUser(l->params[0]))
            U = (user*)getChannel(l->params[0]);
        else
            U = getUser(l->params[0]);
        if(l->paramCount == 3){
            changeMode(&U->modeMinor, &U->modeMajor, l->params[2]);
        } /* TODO: the last parameter */
/*FMODE (644AAAAAA) +4:
	param[0]: #lobby
	param[1]: 1325526922
	param[2]: +h
	param[3]: 644AAAAAC*/
    } else if(!strcmp(l->command,"TOPIC")){
/*TOPIC (644AAAAAA) +1:
	param[0]: #lobby
	text: hi*/
    } else if(!strcmp(l->command,"FTOPIC")){
/*FTOPIC (644) +3:
	param[0]: #lobby
	param[1]: 1325533724
	param[2]: Yellow!Amarillo@netadmin.pokesplash.net
	text: hi*/
    } else if(!strcmp(l->command,"PART")){
        fire_event(EVENT_PART,l);
        delChannelUser(getChannel(l->params[0]),getUser(l->id));
/*PART (644AAAAAC) +1:
	param[0]: #lobby
	text: Removed by Yellow: No reason given*/
    } else if(!strcmp(l->command,"OPERTYPE")){
/*OPERTYPE (644AAAAAA) +1:
	param[0]: NetAdmin*/
    } else if(!strcmp(l->command,"FHOST")){
/*FHOST (644AAAAAA) +1:
	param[0]: netadmin.pokesplash.net*/
    } else if(!strcmp(l->command,"SAVE")){
/*SAVE (644) +2:
	param[0]: 4CEAAAAAB
	param[1]: 1325633878*/
    } else {
        printLine(l);
    }
}

char *generateUIDInsp(){
    int pos = 5;
    while(1){
        cuid[3+pos] += 1;
        if(cuid[3+pos]=='['){
            cuid[3+pos]='0';
            break;
        } else if(cuid[3+pos]==':'){
            cuid[3+pos]='A';
            pos--;
            continue;
        } else {
            break;
        }
    }
    return cuid;
}

user *createUserInsp(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes){
    char buff[512];
    sprintf(buff,":%s UID %s %d %s %s %s %s %s %d %s :%s\r\n",
        getConfigValue("ServerId"),uid,(int)time(NULL),nick,host,host,ident,ip,(int)time(NULL),modes,gecos);
    send_raw_line(buff);
    return addUser(uid, nick, ident, host, ip, vhost, gecos, modes);
}

char isValidNickInsp(char *nick){
    /* TODO: national_chars support */
    int i, nicklen;
    char c, *validlen;
    nicklen = strlen(nick);
    if((validlen = getRemoteConfValue("NICKMAX"))){
        if(nicklen > atoi(validlen)){
            return 0;
        }
    }
    for(i=0;i<nicklen;i++){
        c = nick[i];
        if((c<58)&&(c>47)){
            if(!i)
                return 0;
            else
                continue;
        }
        if((c<97)&&(c>93))
            continue;
        c &= 0x5f;/* lowercase */
        if((c<91)&&(c>64))
            continue;
        if((c<94)&&(c>90))
            continue;
        return 0;
    }
    return 1;
}

void setMode(char *senderid, char *target, char *modes){
    char buff[512];
    user *U;
    if(!senderid||!target||!modes)
        return;
    sprintf(buff,":%s MODE %s %s\r\n",senderid, target, modes);
    send_raw_line(buff);
    if(!isUser(target))
        U = (user*)getChannel(target);
    else
        U = getUser(target);
    changeMode(&U->modeMinor, &U->modeMajor, modes);
    aclog(LOG_DEBUG,"%s set modes %s on %s.\n", senderid, modes, target);
}

void lolping(int argc, char **argv){
    printf("%d lol: %s\n",(int)time(NULL),argv[0]);
}

void printchanmap(int argc, char **argv){
    channode *cnode;
    statusnode *unode;
    cnode = chanlist;
    printf("p: %p\n", cnode);
    while(cnode){
        printf("%s:\n",cnode->C->name);
        unode = cnode->C->users;
        while(unode){
            printf("\t%s\n",unode->U->nick);
            unode = unode->next;
        }
        cnode = cnode->next;
    }
}

void INIT_MOD(){
    remoteconf = NULL;
    send_raw_line = &send_raw_insp;
    parseLine = &parseLineInsp;
    connectIRC = &connectInsp;
    eventloopIRC = &eventloopInsp;
    handleLine = &handleLineInsp;
    generateUID = &generateUIDInsp;
    createUser = &createUserInsp;   
    isValidNick = &isValidNickInsp;
    /*addTimerEvent(printchanmap, time(NULL)+5, 0);
    addTimerEvent(printchanmap, time(NULL)+45, 0);
    addTimerEvent(lolping, time(NULL)+30, 1, "something");
    addTimerEvent(lolping, time(NULL)+300, 1, "something else");*/
}
