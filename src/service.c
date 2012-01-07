#include <actypes.h>
#include <config.h>
#include <acuity.h>
#include <services.h>
#include <events.h>
#include <ircd.h>

user *createService(char *nick, char *host, char *ident, char *gecos){
    char *uid;
    user *U;
    if(!isValidNick(nick)){
        aclog(LOG_ERROR, "Couldn't create service with the invalid nick: %s\n",nick);
        return NULL;
    }
    uid = generateUID();
    U = createUser(uid, nick, ident, host, "0.0.0.0", host, gecos, getConfigValue("ServicesModes"));
    return U;
}
