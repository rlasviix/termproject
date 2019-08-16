#include <string.h>
#include "HBM.h"

using namespace std;

int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	string s;
	int SID, BA;
	int RA;
	int CA;
	int i = 0;
	string cmd;
	
	int finish = 1;
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
		out.width(4);
		out << i++ << "   BANK = " << BA << "   RA = " << RA << "   ";
		finish = hbm->work(BA, RA, CA, cmd, out);
		out << endl;
	}

	delete hbm;

	return 0;
}

