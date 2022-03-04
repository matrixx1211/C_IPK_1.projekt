AUTHOR = xbitom00
# proměnné kompilátoru
CC = gcc
FILES = main.c
CFLAGS = -werror -g

hinfosvc: $(FILES)
	$(CC) $(FILES) -o $@

clean:
	rm hinfosvc *.o $(AUTHOR).zip

zip: Makefile main.c Readme.md
	zip $(AUTHOR) $^ 
