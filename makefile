CC=gcc
CFLAGS=-Wall -g
BINS= libmyalloc.so

all: $(BINS)
libmyalloc.so:  allocator.c
	$(CC) $(CFLAGS) -fPIC -shared allocator.c -o libmyalloc.so list.c -lm

clean:
			rm $(BINS)
%: %.c
					$(CC) $(CFLAGS) -o $@ $<
project:
		rm -f project3.tgz
		tar cvzf project3.tgz makefile allocator.c  README list.c list.h
