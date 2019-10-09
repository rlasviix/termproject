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

int main() {
	int time;
	string address;
	int BA;
	int RA;
	int CA;
	int tick = 0;
	bool finish = true;
	string cmd;
	
	HBM* hbm = new HBM();

	ifstream in("in.txt");
	ofstream out("out.txt");

	struct input
	{
		int time;
		string address;
		string cmd;
	};
	queue<input> job;



	if (!in.is_open()) {
		cout << " no file" << endl;
		return 0;
	}

	while (in.peek() != EOF) {
		in >> time;
		in >> address;
		in >> cmd;
		job.push({ time, address, cmd });
	}

	while (!finish || !job.empty()) {
		if (finish) {
			time = job.front().time;
			if (time <= tick) {
				address = job.front().address;
				mapping(address, &BA, &RA, &CA);
				cmd = job.front().cmd;
				job.pop();
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
		finish = hbm->work(BA, RA, CA, cmd, out);
		out << endl;
	}
	in.close();
	out.close();
	delete hbm;

	return 0;
}