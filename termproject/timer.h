#pragma once
#include<limits.h>
#include<algorithm>
#include <queue>

using namespace std;

class Timer {
public:
	Timer() {
		wait_counter = 0;
		time = 0;
		std::fill_n(cACT, 16, INT_MAX);
	}
	int wait_counter;
	int time;
	int cACT[16];
	queue<int> act;

	void tick();
};