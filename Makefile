shell: main.c
	gcc -o shell main.c shell.c

clean:
	rm shell