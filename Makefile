# Edit here as necessary:
CC = gcc
CFLAGS = -g -Wall -Iincludes -DDEVEL
MODFLAGS = -fpic -rdynamic -export-dynamic
BINFLAGS = -g -Wall -rdynamic
# BSD users:
# LIBS = -lsqlite3
LIBS = -ldl -lsqlite3
SOEXT = .so

# Don't edit below here:

@all: services

services:
	mkdir -p build
	$(CC) $(CFLAGS) -c -Iincludes -o build/acuity.o src/acuity.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/log.o src/log.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/panic.o src/panic.c
	$(CC) $(CFLAGS) -DMODEXT=$(SOEXT) -c -Iincludes -o build/module.o src/module.c
	$(CC) $(BINFLAGS) -o acuity build/acuity.o build/log.o build/panic.o $(LIBS)

cmd = $(CC) $(CFLAGS) $(MODFLAGS) -Iincludes -c -o $(patsubst src/modules/%.c, build/modules/%.o, $(var)) $(var);\
$(CC) $(BINFLAGS) -shared -export-dynamic -o $(patsubst src/modules/%.c, modules/%$(SOEXT), $(var)) $(patsubst src/modules/%.c, build/modules/%.o, $(var));\
echo "Built module: $(patsubst src/modules/%.c, %, $(var))";

mod:
	mkdir -p build/modules
	mkdir -p modules
	@$(foreach var, $(wildcard src/modules/*.c), $(cmd))

clean:
	rm -rf build
	rm -rf modules
	rm -f acuity
