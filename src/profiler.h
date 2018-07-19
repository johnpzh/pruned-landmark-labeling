/*
 * Johnpzh
 * To profile the speed of the labeling process.
 */

#pragma once
#include <sys/time.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class Profiler {
private:
	double start_time;
	unsigned bfs_count;
	unsigned long label_count;

	double get_wtime() const
	{
		timeval t;
		gettimeofday(&t, NULL);
		return t.tv_sec + t.tv_usec * 1E-6;
	}

public:
	Profiler() {
		reset();
	}

	void reset()
	{
		start_time = get_wtime();
		bfs_count = 0;
		label_count = 0;
	}

	double time_click() const
	{
		return get_wtime() - start_time;
	}

	unsigned bfs_click()
	{
		++bfs_count;
		return bfs_count;
	}

	unsigned get_bfs_count()
	{
		return bfs_count;
	}

	void add_label(unsigned long l)
	{
		label_count += l;
	}

	unsigned long get_label_count()
	{
		return label_count;
	}

	template <typename T>
	void print(const T& arg)
	{
		std::cout << arg;
	}

	template <typename First_t, typename... Types>
	void print(const First_t& first, const Types&... args)
	{
		std::cout << first;
		Profiler::print(args...);
	}

	template <typename T>
	bool equal_values(const T& a, const T& b)
	{
		if (a == b)
			return true;
		else
			return false;
	}
};
