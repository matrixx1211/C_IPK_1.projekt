CC = gcc
FILES = main.c
CFLAGS = -werror -g

hinfosvc: $(FILES)
	$(CC) $(FILES) -o $@

clean:
	rm hinfosvc .o