CC:=cc

INCS:=
LIBS:=

CPPFLAGS := 
CFLAGS   := -std=c99 -D_POSIX_C_SOURCE=200809L -pedantic -Wall ${INCS} ${CPPFLAGS} -fsanitize=address -g
LDFLAGS  := ${LIBS} -fsanitize=address # -static-libasan

SRC += fthc.c
OBJ = ${SRC:.c=.o}


.c.o:
	${CC} -c ${CFLAGS} $<

fthc: $(OBJ)
	$(CC) -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f fthc ${OBJ}
