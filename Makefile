INCLUDES= -I include
CXXFLAGS= -pedantic -Wall -Wextra $(INCLUDES) -DFLUENT_MT -Weffc++ -std=c++11 -g

fluent_test: src/test.o src/fluent.o src/socket.o
	$(CXX) src/test.o src/fluent.o src/socket.o -o fluent_test

src/fluent.o: src/fluent.cpp include/fluent_cpp.h include/socket.h
	$(CXX) $(CXXFLAGS) src/fluent.cpp -c -o src/fluent.o

src/socket.o: src/socket.cpp include/socket.h
	$(CXX) $(CXXFLAGS) src/socket.cpp -c -o src/socket.o

src/test.o: src/test.cpp include/fluent_cpp.h
	$(CXX) $(CXXFLAGS) src/test.cpp -c -o src/test.o

.PHONY: test
test: fluent_test
	python run_tests.py

.PHONY: clean
clean:
	rm fluent_test src/*.o
