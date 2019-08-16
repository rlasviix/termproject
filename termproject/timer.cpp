

#include "timer.h"

using namespace std;

void Timer::tick() {
	wait_counter++;
//	if (cRD >= 0 && cRD < INT_MAX) cRD++;
//	if (cWR >= 0 && cWR < INT_MAX) cWR++;
	for (int i = 0; i < 16; i++) {
		if (cACT >= 0 && cACT[i] < INT_MAX) cACT[i]++;
//		if (cPRE >= 0 && cPRE[i] < INT_MAX) cPRE[i]++;
	}
}