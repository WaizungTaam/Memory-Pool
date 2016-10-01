memory_pool_demo.o: memory_pool.h memory_pool_demo.cc
	g++ -std=c++11 memory_pool.h memory_pool_demo.cc -o memory_pool_demo.o