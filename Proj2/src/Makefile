# bathroom version
VERSION = 0

NAME_SERVER = Q2
SRC_SERVER = Q2.c init.c logs.c utls.c vector.c
OBJ_SERVER = ${SRC_SERVER:.c=.o}

NAME_CLIENT = U2
SRC_CLIENT = U2.c init.c logs.c utls.c
OBJ_CLIENT = ${SRC_CLIENT:.c=.o}

# compiler and linker
CC = gcc
# flags
CFLAGS = -Wall

all: options bathroom

options:
	@echo bathroom build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "CC     = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

bathroom: ${OBJ_SERVER} ${OBJ_CLIENT}
	${CC} ${CFLAGS} -o ${NAME_SERVER} ${OBJ_SERVER} -l pthread
	${CC} ${CFLAGS} -o ${NAME_CLIENT} ${OBJ_CLIENT} -l pthread

clean:
	rm -f ${NAME_SERVER} ${OBJ_SERVER} ${NAME_CLIENT} ${OBJ_CLIENT}

.PHONY: all options clean bathroom
