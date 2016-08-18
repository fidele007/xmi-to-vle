CC = g++
CFLAGS = -g -Wall -std=c++11
LDFLAGS = -lboost_filesystem -lboost_system

all: converter

converter: ModelGetter.o ReadXMI.o WriteVPZ.o
	$(CC) $(CFLAGS) -o xmi2vle main.cpp ModelGetter.o ReadXMI.o WriteVPZ.o $(LDFLAGS)

ModelGetter.o: ModelGetter.cpp ModelGetter.hpp
	$(CC) $(CFLAGS) -c ModelGetter.cpp

ReadXMI.o: ReadXMI.cpp ReadXMI.hpp ModelGetter.hpp
	$(CC) $(CFLAGS) -c ReadXMI.cpp

WriteVPZ.o: WriteVPZ.cpp WriteVPZ.hpp
	$(CC) $(CFLAGS) -c WriteVPZ.cpp

clean:
	$(RM) xmi2vle *.o *~
