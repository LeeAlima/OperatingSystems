a.out: test.o threadPool.o osqueue.o
	gcc -o a.out test.o threadPool.o osqueue.o -pthread

test.o: test.c threadPool.h
	gcc -c -o test.o test.c -pthread

threadPool.o: threadPool.c threadPool.h
	gcc -c -o threadPool.o threadPool.c -pthread

osqueue.o: osqueue.c osqueue.h
	gcc -c -o osqueue.o osqueue.c
