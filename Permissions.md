# Permissions List #

## Special ##

**sp.EVERYTHING** (implies _all_ permissions)

**sp.override** (implies all F**commands on all services)**

## NickServ ##

**ns** (access to nickserv)

**ns.auspex** (e.g., seeing full info output for other nicks)

**ns.set:**
  * ns.set.email
  * ns.set.password
  * ns.set.nick
  * ns.set.time
  * ns.set.kill

## OperServ ##

(Opers can generally see all operserv things: perm list, news list, akill list, ...)

**os** (access to operserv)

**os.perm.chg** (Able to perm set and perm addclass)

**os.mod** (Able to load and unload modules on the fly)

**os.global** (Able to send globals)

**os.news** (Able to change opernews/logonnews)

**os.servers** (Able to jupe servers, rehash servers remotely if IRCd allows, ...)

**os.admin** (Able to os rehash/die)

**os.ban** (Able to os akill and similar)