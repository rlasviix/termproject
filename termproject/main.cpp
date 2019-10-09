#include <string.h>
#include <sstream>
#include "HBM.h"

#define ba_width 4
#define ra_width 14
#define ca_width 6

//#define ba_width 4
//#define ra_width 12
//#define ca_width 4

using namespace std;

void mapping(string address, int* BA, int* RA, int* CA) {
	long tempA, tempB;
	long physicalAddress;
	string str;
	address = address.erase(0, 2);
	stringstream convert(address);
	convert >> hex >> physicalAddress;

	tempA = physicalAddress;
	physicalAddress = physicalAddress >> ca_width;
	tempB = physicalAddress << ca_width;
	*CA = tempA ^ tempB;

	tempA = physicalAddress;
	physicalAddress = physicalAddress >> ra_width;
	tempB = physicalAddress << ra_width;
	*RA = tempA ^ tempB;

	tempA = physicalAddress;
	physicalAddress = physicalAddress >> ba_width;
	tempB = physicalAddress << ba_width;
	*BA = tempA ^ tempB;
}

template<typename T>
bool next_cmd_check(deque<T> job, int BA, int RA, int tick) {
	int i = 0;
	if (job.empty()) return false;
	while (job.at(i).time <= tick) {
		if (job.at(i).BA == BA) {
			if (job.at(i).RA != RA) return true;
			else return false;
		}
		if (job.size() > i + 1) i++;
		else return false;
	}
	return false;
}

int main() {
	int time;
	string address;
	int BA;
	int RA;
	int CA;
	int tick = 0;
	bool finish = true;
	bool pre = false;
	string cmd;
	
	HBM* hbm = new HBM();

	ifstream in("in.txt");
	ofstream out("out.txt");

	struct input
	{
		int time;
		int BA;
		int RA;
		int CA;
		string cmd;
	};
	deque<input> job;



	if (!in.is_open()) {
		cout << " no file" << endl;
		return 0;
	}

	while (in.peek() != EOF) {
		in >> time;
		in >> address;
		mapping(address, &BA, &RA, &CA);
		in >> cmd;
		job.push_back({ time, BA, RA, CA, cmd });
	}

	while (!finish || !job.empty()) {
		if (finish) {
			time = job.front().time;
			if (time <= tick) {
				BA = job.front().BA;
				RA = job.front().RA;
				CA = job.front().CA;
				cmd = job.front().cmd;
				job.pop_front();
				pre = next_cmd_check(job, BA, RA, tick);
				finish = 0;
			}
			else {
				cmd = "NOP";
			}
		}
		out <<"tick";
		out.width(3);
		out << tick++ << "   BANK = ";
		out.width(3);
		out << BA << "   RA = ";
		out.width(3);
		out << RA << "   ";
		
		finish = hbm->work(pre, BA, RA, CA, cmd, out);
		out << endl;
	}
	in.close();
	out.close();
	delete hbm;

	return 0;
}