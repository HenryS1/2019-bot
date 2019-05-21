FLAGS=-fpermissive -std=c++17

read_data.o: read_data.cpp read_data.hpp
	g++ $(FLAGS) read_data.cpp -c

test: data.hpp tests.cpp read_data.o
	g++ $(FLAGS) -lgtest read_data.o tests.cpp -o tests
	./tests
