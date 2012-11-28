CC=gcc
CFLAGS=-O2 -Wall -C99

filetrans: filetrans.c client.c server.c
	${CC} -o filetrans ${CFLAGS} filetrans.c client.c server.c        

debug: filetrans_debug

filetrans_debug: filetrans.c client.c server.c
	${CC} -o filetrans_d -D_DEBUG -g ${CFLAGS} filetrans.c client.c server.c

clean:
	rm -rf filetrans filetrans_d

