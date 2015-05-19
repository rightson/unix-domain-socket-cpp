CXXFLAGS := -pthread -Wall
TARGETS := server.o main.o

server: $(TARGETS)
	$(CXX) -o $@ $^

$(TARGETS): server.hpp

clean:
	rm -f server *.o

.PHONY: clean
