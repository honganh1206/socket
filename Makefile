CC = gcc
CFLAGS = -Wall -Wextra

# Default rule: build any target matching a .c file
%: %.c
	$(CC) $(CFLAGS) $< utils.c -o bin/$@

clean:
	rm -f *.o
	rm -f $(wildcard *)
