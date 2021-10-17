proxy: main.o 
	g++ main.o -o proxy
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm *.o proxy