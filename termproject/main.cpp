#include <string.h>
#include "HBM.h"

using namespace std;

int main() {
	int address;
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
			in >> hex >> BA >> RA >> CA;
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