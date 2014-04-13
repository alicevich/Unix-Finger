all: 
	gcc -o fingerserver fingerserver.c -lpthread
	gcc -o fingerclient fingerclient.c 