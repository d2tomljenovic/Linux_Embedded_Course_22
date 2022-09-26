SHELL = /bin/sh

OBJS = getch.o main.o
CFLAGS = -Wall
CC = ${CROSS_COMPILE}gcc
INCLUDES = -I inc
LIBS = -lpthread
LDFLAGS = -static

all: KonzApp
.PHONY: all

KonzApp:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

.PHONY: clean
clean:
	rm -f *o KonzApp

%.o:%.c
	${CC} ${CFLAGS} ${INCLUDEs} -c $< -o $@
