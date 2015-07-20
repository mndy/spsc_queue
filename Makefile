.PHONY: clean test

CXX := clang++
CXXFLAGS := -std=c++11 -g
CXXINC := -I.
GTESTROOT := ../googletest
GTESTINC := -I$(GTESTROOT) -I$(GTESTROOT)/include
GTESTSRC := $(GTESTROOT)/src/gtest-all.cc  $(GTESTROOT)/src/gtest_main.cc

example: main.o
	$(CXX) $(CXXFLAGS) $(CXXINC) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXINC) -c -o $@ $<

gtest: tests.cpp spsc_queue.h
	$(CXX) $(CXXFLAGS) $(CXXINC) $(GTESTINC) -o $@ $(GTESTSRC) $<

test: gtest
	./gtest

clean:
	rm -f *.o example gtest
