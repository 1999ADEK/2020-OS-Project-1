#CFLAGS = -DDEBUG -Wall
CFLAGS = -Wall
main: main.o schedule.o process.o
	gcc $(CFLAGS) -o main main.o schedule.o process.o
main.o: main.c
	gcc $(CFLAGS) -c main.c
schedule.o: schedule.c schedule.h
	gcc $(CFLAGS) -c schedule.c
process.o: process.c process.h
	gcc $(CFLAGS) -c process.c
clean:
	rm -rf *o
