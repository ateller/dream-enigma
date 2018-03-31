CC=gcc
CFLAGS=-fsanitize=address -g

dream-archiver: dream-enigma.c
	$(CC) -o dream-enigma dream-enigma.c $(CFLAGS)
clean: 
	-rm dream-enigma
