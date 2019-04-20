read_data.o: read_data.cpp read_data.hpp
	g++ -std=c++17 -lgtest read_data.cpp -c

test: data.hpp tests.cpp read_data.o
	g++ -std=c++17 -lgtest read_data.o tests.cpp -o tests
	./tests
