.PHONY: clean test

GTESTROOT ?= ../../googletest
CXX       ?= clang++
CXXFLAGS  ?= -std=c++11 -g
CXXINC     = -I../include
GTESTINC   = -I$(GTESTROOT) -I$(GTESTROOT)/include
GTESTSRC   = $(GTESTROOT)/src/gtest-all.cc  $(GTESTROOT)/src/gtest_main.cc

test: gtest.O3.out gtest.O2.out gtest.O0.out
	./gtest.O0.out
	./gtest.O2.out
	./gtest.O3.out

gtest.%.out: tests.cpp ../include/spsc_queue.h
	$(CXX) $(CXXFLAGS) -$* $(CXXINC) $(GTESTINC) -o $@ $(GTESTSRC) $<

clean:
	rm -rf *.out *.dSYM
