CC = gcc
CFLAGS = -g -Wall -Iincludes
MODFLAGS = -fpic -rdynamic -DMODULE -export-dynamic
LIBS = -ldl

@all:
	$(CC) $(CFLAGS) -c -Iincludes -o build/acuity.o src/acuity.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/config.o src/config.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/module.o src/module.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/events.o src/events.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/encrypt.o src/encrypt.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/service.o src/service.c
	$(CC) $(CFLAGS) -c -Iincludes -o build/util.o src/util.c
	$(CC) -g -Wall -rdynamic -o acuity build/acuity.o build/config.o build/module.o build/service.o build/events.o build/util.o build/encrypt.o $(LIBS)

#config:
#	gcc -c -fpic -DMODULE -o build/config.o -Iincludes config.c
#	gcc -c -fpic -DMODULE -o build/defaults.o -Iincludes src/defaults.c
#	gcc -shared -o modules/config.so build/config.o build/defaults.o

#modules:
#	gcc -g -c -fpic -rdynamic -DMODULE -o build/inspircd20.o -Iincludes src/inspircd20.c
#	gcc -g -shared -rdynamic -o modules/inspircd20.so build/inspircd20.o


cmd = $(CC) $(CFLAGS) $(MODFLAGS) -Iincludes -c -o $(patsubst src/modules/%.c, build/modules/%.o, $(var)) $(var);\
$(CC) -g -Wall -shared -rdynamic -export-dynamic -o $(patsubst src/modules/%.c, modules/%.so, $(var)) $(patsubst src/modules/%.c, build/modules/%.o, $(var));\
echo "Built module: $(patsubst src/modules/%.c, %, $(var))";

mod:
	@$(foreach var, $(wildcard src/modules/*.c), $(cmd))

# vim: set shiftwidth=4 softtabstop=4:
