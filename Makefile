# Edit here as necessary:
CC = gcc
CFLAGS = -g -Wall -Iincludes -DDEVEL
MODFLAGS = -fpic -rdynamic -export-dynamic
BINFLAGS = -g -Wall -rdynamic
# BSD users:
# LIBS = -lsqlite3
LIBS = -ldl -lsqlite3

# Don't edit below here:

@all: services mod

services:
	mkdir -p build
	$(CC) $(CFLAGS) -c -Iincludes -o build/acuity.o src/acuity.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/panic.o src/panic.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/config.o src/config.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/db.o src/db.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/module.o src/module.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/events.o src/events.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/service.o src/service.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/util.o src/util.c
	$(CC) $(BINFLAGS) -o acuity build/acuity.o build/panic.o build/config.o build/db.o build/module.o build/service.o build/events.o build/util.o build/encrypt.o $(LIBS)

cmd = $(CC) $(CFLAGS) $(MODFLAGS) -Iincludes -c -o $(patsubst src/modules/%.c, build/modules/%.o, $(var)) $(var);\
$(CC) $(BINFLAGS) -shared -export-dynamic -o $(patsubst src/modules/%.c, modules/%.so, $(var)) $(patsubst src/modules/%.c, build/modules/%.o, $(var));\
echo "Built module: $(patsubst src/modules/%.c, %, $(var))";

mod:
	mkdir -p build/modules
	mkdir -p modules
	@$(foreach var, $(wildcard src/modules/*.c), $(cmd))
