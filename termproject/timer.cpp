

#include "timer.h"

using namespace std;

void Timer::tick() {
	wait_counter++;
	time++;
	for (int i = 0; i < 16; i++) {
		if (cACT >= 0 && cACT[i] < INT_MAX) cACT[i]++;
	}
}