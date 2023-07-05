objects = termcomm.o comm.o

termcomm : $(objects)
	g++ -o  termcomm $(objects) -pthread

termcomm.o : termcomm.cpp termcomm.h
	g++ -c termcomm.cpp 

comm.o : comm.cpp comm.h
	g++ -c comm.cpp

run:
	./termcomm

clean :
	rm libtest $(objects)

