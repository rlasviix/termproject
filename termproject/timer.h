#pragma once
#include<limits.h>
#include<algorithm>

using namespace std;

class Timer {
public:
	Timer() {
		wait_counter = 0;
//		cRD = INT_MAX; cWR = INT_MAX;
		std::fill_n(cACT, 16, INT_MAX);
//		std::fill_n(cPRE, 16, INT_MAX);
	}
	int wait_counter;
//	int cRD, cWR;
	int cACT[16];
//	int cPRE[16];

	void tick();
};