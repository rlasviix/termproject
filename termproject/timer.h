#pragma once
#include<limits.h>
#include<algorithm>
#include <queue>

using namespace std;

class Timer {
public:
	Timer() {
		time = 0;
	}
	int time;
	queue<int> act;

	void tick();
};