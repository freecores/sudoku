CXX := arm-linux-gnueabi-g++
CXXFLAGS := -static -O3

sudoku-search: main.cc driver.h counters.h counters.cc
	$(CXX) $(CXXFLAGS) $^ -o $@
