CXX=g++
CXXFLAGS=-O3 -std=c++0x -Wall -pthread
LDLIBS=-lgmp

pollardRho: iFactor.cpp
	$(CXX) -o iFactor iFactor.cpp $(CXXFLAGS) $(LDLIBS)
