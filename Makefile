CXX = g++
CXXFLAGS = -g -Wall -Wextra -O3 -std=c++1y

# By Johnpzh
LIB = -I./include -L./lib -lNetworKit -fopenmp
# End By Johnpzh

all: bin bin/construct_index bin/query_distance bin/benchmark

bin:
	mkdir -p bin

bin/construct_index: samples/construct_index_main.cc src/pruned_landmark_labeling.h
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $^ $(LIB)

bin/query_distance: samples/query_distance_main.cc src/pruned_landmark_labeling.h
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $^ $(LIB)

bin/benchmark: samples/benchmark_main.cc src/pruned_landmark_labeling.h
	$(CXX) $(CXXFLAGS) -Isrc -o $@ $^ $(LIB)

bin/pruned_landmark_labeling_test: src/pruned_landmark_labeling_test.cc src/pruned_landmark_labeling.h
	$(CXX) $(CXXFLAGS) -lgtest -lgtest_main -o $@ $^ $(LIB)



.PHONY: test clean

test: bin/pruned_landmark_labeling_test
	bin/pruned_landmark_labeling_test

clean:
	rm -rf bin
