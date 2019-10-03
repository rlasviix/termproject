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
	string address;
	int BA;
	int RA;
	int CA;
	int i = 0;
	bool finish = true;
	string cmd;
	
	HBM* hbm = new HBM();

	ifstream in("in.txt");
	ofstream out("out.txt");


	if (!in.is_open()) {
		cout << " no file" << endl;
		return 0;
	}

	while (!finish || in.peek() != EOF) {
		if (finish) {
			in >> address;
			mapping(address, &BA, &RA, &CA);
			//in >> hex >> BA >> RA >> CA;
			in >> cmd;
			finish = 0;
		}
		out <<"tick";
		out.width(3);
		out << i++ << "   BANK = ";
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