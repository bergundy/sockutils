CC=gcc
FLAGS=-Wall
PREFIX=/usr

all: libsockutils.so

libsockutils.so: sockutils.o sockutils.h
	$(CC) $(FLAGS) -shared -o libsockutils.so sockutils.o

sockutils.o: sockutils.c sockutils.h
	$(CC) $(FLAGS) -c sockutils.c

clean:
	rm -f *.o *.so

install: sockutils.h libsockutils.so
	cp sockutils.h /usr/include
	cp libsockutils.so /usr/lib

uninstall:
	rm -f $(PREFIX)/include/sockutils.h 
	rm -f $(PREFIX)/lib/libsockutils.so 
