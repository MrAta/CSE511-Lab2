LINK=gcc
CC=gcc
CFLAGS=-c -ggdb -Wall -I. -I/opt/local/include -I./libdict/include
LINKFLAGS=-L. -g -ggdb -pthread -L/opt/local/lib -L./libdict/bin
LIBFLAGS=-shared -Wall -pthread
LINKLIBS=-lm -lrt -ldict
TARGETS=server client
OBJECTS= server-main.o \
			db.o \
			cache.o \
			lsm.o \
			server-part1.o

all: $(TARGETS)

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

server: $(OBJECTS)
	$(LINK) $(LINKFLAGS) $^ $(LINKLIBS) -o $@

client: client.c
	gcc -g -std=gnu99 client.c -o client -pthread -lm

client7: client7.c
	gcc -g -std=gnu99 client7.c -o client7 -pthread -lm

client2: client2.c
	gcc -g -std=gnu99 client2.c -o client2 -pthread

client3: client3.c
	gcc -g -std=gnu99 client3.c -o client3 -pthread

client4: client4.c
	gcc -g -std=gnu99 client4.c -o client4 -pthread

client5: client5.c
	gcc -g -std=gnu99 client5.c -o client5 -pthread

client6: client6.c
	gcc -g -std=gnu99 client6.c -o client6 -pthread

client8: client8.c
	gcc -g -std=gnu99 client8.c -o client8 -pthread -lm

clean:
	rm server client client2 client3 client4 client5 client6 client7 client8 *.o
