# simpledu version
VERSION = 0
NAME = simpledu

SRC = init.c logs.c sigs.c simpledu.c utls.c
OBJ = ${SRC:.c=.o}

# compiler and linker
CC = gcc
# flags
CFLAGS = -pedantic -Wall -Wextra -O2

all: options simpledu

options:
	@echo simpledu build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "CC     = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

simpledu: ${OBJ}
	${CC} ${CFLAGS} -o ${NAME} ${OBJ}

clean:
	rm -f ${NAME} ${OBJ}

.PHONY: all options clean simpledu
