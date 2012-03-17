#include <services.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

static acsocket irc_socket;

void send_raw(const char *buff){
    write(irc_socket, buff, strlen(buff));
}

int irc_connect(char *host, char *port){
    struct addrinfo hints, *res = NULL;
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
        }else if(hints.ai_family == AF_INET6){
            hints.ai_family = AF_INET;
        }else{
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
        aclog(LOG_ERROR, "Cannot connect to IRCd. Error: %d\n", err);
        return 0;
    }
    return 1;
}

int irc_eventloop(){
    char buff[1024], *buffptr, *c, *tok = NULL, isFrag;
    line_t line;
    int len, i;

    if(!irc_socket)
        return -1;
    for(i=0; i < 1024; i++) buff[i] = 0;
    buffptr = buff;
    while((len = read(irc_socket, buffptr, 512))){
        *(buffptr+len) = '\0';
        isFrag = buff[strlen(buff)-1];
        isFrag = ((isFrag != '\n')&&(isFrag != '\r'));
        buffptr = strtok_r(buff, "\r\n", &tok);
        while(1){
            c = strtok_r(NULL, "\r\n", &tok);
            if(isFrag&&(!c)) break;
            line = parseLine(buffptr);
            handleLine(&line);
            freeLine(&line, 1);
            if(!c) break;
            buffptr = c;
        }
        if(isFrag){
            c = buff;
            do {
                c[0] = buffptr[0];
                buffptr++;
                c++;
            }while(buffptr[0] != '\0');
            buffptr = c;
            continue;
        }
        buffptr = buff;
        fire_timer_event();
        /* fire every time buffer is clear */
    }
    return 0;
}

void irc_close(char *reason){
    handleClose(reason);
    shutdown(irc_socket, SHUT_RDWR);
    close(irc_socket);
}
