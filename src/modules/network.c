#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <acuity.h>
#include <network.h>
#include <entity.h>


int _char2mode(char modechar){
    /* converts 'A' (or 'a') to MODE_A */
    return 1<<((modechar&0x5F)-0x41);
}

char *buildModes(int count, ...){
    char *modes, modechar;
    va_list args;
    int addPos = 1, remPos, i, j, arg, mode;
    remPos = count+1;
    /* +ABC-DEF; addPos = A (1), remPos = F (7) */
    va_start(args,count);
    safenmalloc(modes, char, count+2, NULL);
    for(i = 0; i < count; i++){
        arg = va_arg(args, int);
        mode = (arg&MODE_CHAR);
        j = 0;
        while(1<<j != mode) j++;
        modechar = j+0x41;
        if(!(arg&MODE_MAJOR))
            modechar |= 0x20;
        if(arg&MODE_REMOVE){
            modes[remPos--] = modechar;
        } else {
            modes[addPos++] = modechar;
        }
    }
    if(remPos == count+1)
        modes[remPos] = '\0';/* stop at - sign */
    else
        modes[remPos] = '-';
    if(addPos == 1){
        for(i=0; i < count+2; i++){
            modes[i] = modes[i+1];
        }/* shift entire thing down */
    }else{
        modes[0] = '+';
    }
    va_end(args);
    return modes;
}

char checkModes(int *modeM, int count, ...){
    /* modeM should be &U->modeMinor */
    va_list args;
    int i, mode, arg;
    if(!modeM)
        return 0;
    for(i = 0; i < count; i++){
        arg = va_arg(args, int);
        mode = (arg&MODE_CHAR);
        if(arg&MODE_REMOVE){
            if((arg&MODE_MAJOR)&&(modeM[1]&mode)){
                return 0;
            } else if(!(arg&MODE_MAJOR)&&(modeM[0]&mode)){
                return 0;
            } else {
                continue;
            }
        }else{
            if((arg&MODE_MAJOR)&&(modeM[1]&mode)){
                continue;
            } else if(!(arg&MODE_MAJOR)&&(modeM[0]&mode)){
                continue;
            } else {
                return 0;
            }
        }
    }
    return 1;
}

user *_addUser(char *uid, char *nick, char *ident, char *host, char *ip, char *vhost, char *gecos, char *modes){
    /* all data is copied so that anything can be erased without this structure being disturbed. It will clean up after itself later */
    user *U;
    usernode *node, *next;
    int i, modelen;
    U = (user*)malloc(sizeof(user));
    if(!U){
        aclog(LOG_ERROR,"Failed to create user. Could not allocate memory.\n");
        return NULL;
    }
#define COPYARG2USER(field) do { \
    U-> field = (char*)malloc(sizeof(char)*(strlen( field )+1)); \
    if(!U-> field ){ \
        aclog(LOG_ERROR,"Failed to create user. Could not allocate memory for %s.\n", #field ); \
        return NULL; \
    } \
    strcpy(U-> field , field ); \
} while(0)
    COPYARG2USER(uid);
    COPYARG2USER(nick);
    COPYARG2USER(ident);
    COPYARG2USER(host);
    COPYARG2USER(ip);
    COPYARG2USER(vhost);
    COPYARG2USER(gecos);
#undef COPYARG2USER
    U->modeMinor = 0;
    U->modeMajor = 0;
    modelen = strlen(modes);
    for(i=0; i<modelen; i++){
        if(modes[i]=='+')
            continue;
        if(modes[i]&0x20)
            U->modeMinor |= char2mode(modes[i]);
        else
            U->modeMajor |= char2mode(modes[i]);
    }
    U->chans = NULL;
    U->metadata = NULL;
    if(!userlist){
        userlist = (usernode*)malloc(sizeof(usernode));
        if(!userlist){
            aclog(LOG_ERROR,"Failed to create userlist after creating user. Could not allocate memory.\n");
            return NULL;
        }
        userlist->U = U;
        userlist->next = NULL;
    } else {
        node = userlist;
        while(node->next) node = node->next;
        next = (usernode*)malloc(sizeof(usernode));
        if(!next){
            aclog(LOG_ERROR,"Failed to create usernode after creating user. Could not allocate memory.\n");
            return NULL;
        }
        next->U = U;
        next->next = NULL;
        node->next = next;
    }
    aclog(LOG_DEBUG,"Introduced user %s.\n",U->uid);
    
    return U;
}

void changeMode(int *modeM, char *modes){
    char isAdding = 1;
    while(modes[0]){
        if(modes[0] == '+'){
            isAdding = 1;
        }else if(modes[0] == '-'){
            isAdding = 0;
        }else if(modes[0]&0x20){
            if(isAdding)
                modeM[0] |= char2mode(modes[0]);
            else
                modeM[0] &= ~char2mode(modes[0]);
        }else{
            if(isAdding)
                modeM[1] |= char2mode(modes[0]);
            else
                modeM[1] &= ~char2mode(modes[0]);
        }
        modes++;
    }
}
void changeNick(user *U, char *nick){
    if(!U||!nick)
        return;
    free(U->nick);
    safenmalloc(U->nick, char, strlen(nick)+1, );
    strcpy(U->nick, nick);
}

user *_getUser(char *uid){
    user *U;
    usernode *node;
    
    if(!userlist)
        return NULL;
    node = userlist;
    while(node){
        U = node->U;
        if(!strcmp(uid,U->uid)){
            return U;
        }
        node = node->next;
    }
    return NULL;
}

chan *_addChannel(char *name, char **pmodes){
    chan *C;
    channode *node, *next;
    char *modes;
    int i, modelen;
    if((C = getChannel(name))){
        return C;
    }
    C = (chan*)malloc(sizeof(chan));
    if(!C){
        aclog(LOG_ERROR,"Failed to create channel. Could not allocate memory.\n");
        return NULL;
    }
    C->name = (char*)malloc(sizeof(char)*(strlen(name)+1));
    strcpy(C->name, name);
    C->topic = NULL;
    C->metadata = NULL;
    C->modeMinor = 0;
    C->modeMajor = 0;
    modes = (pmodes++)[0];
    if(modes){
        /* CHANMODES=IXbe,k,FLjl,ACKMNOQRSTcimnprst */
        modelen = strlen(modes);
        for(i=0; i<modelen; i++){
            if(modes[i]=='+')
                continue;
            if(modes[i]&0x20)
                C->modeMinor |= char2mode(modes[i]);
            else
                C->modeMajor |= char2mode(modes[i]);
        }
    }
    C->users = NULL;
    if(!chanlist){
        chanlist = (channode*)malloc(sizeof(channode));
        if(!chanlist){
            aclog(LOG_ERROR,"Failed to create chanlist after creating channel. Could not allocate memory.\n");
            return NULL;
        }
        chanlist->C = C;
        chanlist->next = NULL;
    } else {
        node = chanlist;
        while(node->next) node = node->next;
        next = (channode*)malloc(sizeof(channode));
        if(!next){
            aclog(LOG_ERROR,"Failed to create channode after creating channel. Could not allocate memory.\n");
            return NULL;
        }
        next->C = C;
        next->next = NULL;
        node->next = next;
    }
    aclog(LOG_DEBUG,"Introduced channel %s.\n",C->name);
    return C;
}

chan *_getChannel(char *name){
    chan *C;
    channode *node;
    
    if(!chanlist)
        return NULL;
    node = chanlist;
    while(node){
        C = node->C;
        if(!strcasecmp(name,C->name)){
            return C;
        }
        node = node->next;
    }
    return NULL;
}

statusnode *_addChannelUser(chan *C, user *U){
    statusnode *unode, *uprev;
    channode *cnode, *cprev;

    if((!C)||(!U))
        return NULL;
    unode = (statusnode*)malloc(sizeof(statusnode));
    if(!unode){
        aclog(LOG_ERROR,"Failed to create status node for channel user list. Could not allocate memory.\n");
        return NULL;
    }
    cnode = (channode*)malloc(sizeof(channode));
    if(!cnode){
        aclog(LOG_ERROR,"Failed to create channel node for user channel list. Could not allocate memory.\n");
        return NULL;
    }
    unode->U = U;
    unode->next = NULL;
    unode->modeMinor = 0;
    unode->modeMajor = 0;
    cnode->C = C;
    cnode->next = NULL;
    if(!C->users){
        C->users = unode;
    } else {
        uprev = C->users;
        while(uprev->next) uprev = uprev->next;
        uprev->next = unode;
    }
    if(!U->chans){
        U->chans = cnode;
    } else {
        cprev = U->chans;
        while(cprev->next) cprev = cprev->next;
        cprev->next = cnode;
    }

    return unode;
}

void _delChannelUser(chan *C, user *U){
    channode *cnode, *cprev=NULL;
    statusnode *unode, *uprev=NULL;
    
    if((!C)||(!U))
        return;
    cnode = U->chans;
    while((cnode&&(cnode->C!=C))){ cprev = cnode; cnode = cnode->next; }
    if(!cnode)
        return;
    if(!cprev)
        U->chans = cnode->next;
    else
        cprev->next = cnode->next;
    free(cnode);
    unode = C->users;
    while((unode&&(unode->U!=U))){ uprev = unode; unode = unode->next; }
    if(!unode)
        return;
    if(!uprev)
        C->users = unode->next;
    else
        uprev->next = unode->next;
    free(unode);
}

int _chanStatusAppend(char *channame, char *status){
    /* appends multiple users the channel list from a status string formatted like "o,3AZAAAAAA vh,3AZAAAAAB" */
    int success = 0;
    chan *C;
    user *U;
    char *mode, *uid, continueAfter=1;
    statusnode *snode;

    C = getChannel(channame);
    if(!C)
        return -1;
    mode = status;
    while(continueAfter){
        if(status[0]==','){
            status[0] = '\0';
            uid = status+1;
        } else if((status[0]==' ')||(status[0]=='\0')){
            continueAfter = status[0];
            status[0] = '\0';
            U = getUser(uid);
            snode = addChannelUser(C, U);
            while(mode[0]){
                if(mode[0]&0x20)
                    snode->modeMinor |= char2mode(mode[0]);
                else
                    snode->modeMajor |= char2mode(mode[0]);
                mode++;
            }
            aclog(LOG_DEBUG, "Added %s to %s with mode %d.\n", U->nick, C->name, snode->modeMinor);
            success++;
            mode = status+1;
        }
        status++;
    }

    return success;
}


void INIT_MOD(){
    userlist = NULL;
    chanlist = NULL;
    addUser = &_addUser;
    getUser = &_getUser;
    addChannel = &_addChannel;
    getChannel = &_getChannel;
    addChannelUser = &_addChannelUser;
    delChannelUser = &_delChannelUser;
    chanStatusAppend = &_chanStatusAppend;
    char2mode = &_char2mode;
}
