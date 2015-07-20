.PHONY: clean

CXX      ?= clang++
CXXFLAGS ?= -std=c++11 -g
CXXINC    = -I../../include

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXINC) -c -o $@ $<

simple: main.o
	$(CXX) $(CXXFLAGS) $(CXXINC) -o $@ $^

clean:
	rm -f *.o simple
