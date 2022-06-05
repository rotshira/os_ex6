LD = arm-none-eabi-g++
all: server client main1 Guard singleton activeobject pollServer SelectClient

server: server.cpp
	g++ server.cpp -lpthread -o server

client: client.c
	g++ client.c -o client

pollServer: pollServer.c
	g++ pollServer.c -lpthread -o pollServer

SelectClient: SelectClient.c
	g++ SelectClient.c -lpthread -o SelectClient

Guard: Guard.cpp
	g++ Guard.cpp -lpthread -o Guard

main1: main1.cpp
	g++ main1.cpp -lpthread -o main1

singleton: singleton.cpp
	g++ singleton.cpp -o singleton

activeobject: active_object.cpp
	g++ active_object.cpp -o active_object

# app: serverpoll reactor
#     gcc -o app serverpoll.o
# serverpoll.o: serverpoll.c reactor.h
#     gcc -c serverpoll.c
clean:
	rm -f *.o server client pollServer SelectClient Guard active_object main1 singleton app 

