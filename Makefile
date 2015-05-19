
server: server.cpp server.hpp
	g++ $< -o $@ -pthread

clean:
	rm -f server

.PHONY: clean
