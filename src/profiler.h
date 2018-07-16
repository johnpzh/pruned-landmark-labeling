/*
 * Johnpzh
 * To profile the speed of the labeling process.
 */

#pragma once
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class Profiler {
private:
	double start_time;
	unsigned bfs_count;

	const double get_wtime() const
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
	}

	const double time_click() const
	{
		return get_wtime() - start_time;
	}

	unsigned bfs_click()
	{
		++bfs_count;
		return bfs_count;
	}

	void print(const char *s) const
	{
		printf("%s", s);
	}
};
