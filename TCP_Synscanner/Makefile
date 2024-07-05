PSCANNER: main.o utils.o SynScanner.o 
	g++ main.o utils.o SynScanner.o -o PSCANNER -pthread

main.o: main.cpp
	g++ -c main.cpp

utils.o: utils.cpp
	g++ -c utils.cpp

SynScanner.o: SynScanner.cpp
	g++ -c SynScanner.cpp

clean:
	rm -rf *.o PSCANNER