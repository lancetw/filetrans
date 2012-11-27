CC=gcc
CFLAGS=-O2 -Wall

all: filetrans client

filetrans: filetrans.c
	${CC} -o filetrans ${CFLAGS} filetrans.c          
client: client.c
	${CC} -o client ${CFLAGS} client.c

debug: filetrans_debug client_debug

filetrans_debug: filetrans.c
	${CC} -o filetrans_d -D_DEBUG -g ${CFLAGS} filetrans.c

client_debug: client.c
	${CC} -o client_d -D_DEBUG -g ${CFLAGS} client.c

clean:
	rm -rf filetrans client filetrans_d client_d

