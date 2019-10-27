#include "PIN.h"

using namespace std;



PIN::PIN(){
	output.open("pin.csv", std::ios::binary);
	row_com = Command::NOP;
	col_com = Command::NOP;
}

PIN::~PIN() {
	output.close();
}

void PIN::set_command(int command, int bank_add, int row_add, int col_add) {
	int ba = bank_add;
	int ra = row_add;
	int ca = col_add;
	if (Command(command) == Command::RD || Command(command) == Command::RDA || Command(command) == Command::WR || Command(command) == Command::WRA) {
		row_com = Command::NOP;
		col_com = Command(command);
	}
	else {
		row_com = Command(command);
		col_com = Command::NOP;
	}
	for (int i = 0; i < 4; i++) {
		BA[i] = ba & 1;
		ba = ba >> 1;
	}
	for (int i = 0; i < 14; i++) {
		RA[i] = ra & 1;
		ra = ra >> 1;
	}
	for (int i = 0; i < 6; i++) {
		CA[i] = ca & 1;
		ca = ca >> 1;
	}
}


void PIN::translate_rising() {
	switch (int(row_com)) {
	case (int(Command::NOP)):
		R[0] = 1; R[1] = 1; R[2] = 1;
		break;
	case (int(Command::ACT)):
		if (counter == 0) {
			CKE = 1;
			R[0] = 0; R[1] = 1;
			R[3] = BA[0]; R[4] = BA[1]; R[5] = BA[2];
		}
		else {
			R[0] = RA[5]; 
			R[1] = RA[6]; 
			R[2] = RA[7]; 
			R[3] = RA[8]; 
			R[4] = RA[9];
			R[5] = RA[10];
		}
		break;
	case (int(Command::PRE)):
		CKE = 1;
		R[0] = 1; R[1] = 1; R[2] = 0;
		R[3] = BA[0]; R[4] = BA[1]; R[5] = BA[2];
		break;
	}

	switch (int(col_com)) {
	case (int(Command::NOP)):
		C[0] = 1; C[1] = 1; C[2] = 1;
		break;
	case (int(Command::RD)):
		CKE = 1;
		C[0] = 1; C[1] = 0; C[2] = 1; C[3] = 0;
		C[4] = BA[0]; C[5] = BA[1]; C[6] = BA[2]; C[7] = BA[3];
		break;
	case (int(Command::RDA)):
		CKE = 1;
		C[0] = 1; C[1] = 0; C[2] = 1; C[3] = 1;
		C[4] = BA[0]; C[5] = BA[1]; C[6] = BA[2]; C[7] = BA[3];
		break;
	case (int(Command::WR)):
		CKE = 1;
		C[0] = 1; C[1] = 0; C[2] = 0; C[3] = 0;
		C[4] = BA[0]; C[5] = BA[1]; C[6] = BA[2]; C[7] = BA[3];
		break;
	case (int(Command::WRA)):
		CKE = 1;
		C[0] = 1; C[1] = 0; C[2] = 0; C[3] = 1;
		C[4] = BA[0]; C[5] = BA[1]; C[6] = BA[2]; C[7] = BA[3];
		break;
	}
}

void PIN::translate_falling() {
	switch (int(row_com)) {
	case (int(Command::NOP)):
		break;
	case (int(Command::ACT)):
		if (counter == 0) {
			CKE = 1;
			R[0] = RA[11];
			R[1] = RA[12];
			//R[2] = RA[7);
//			R[3] = BA[4];
			R[4] = RA[13];
			R[5] = BA[3];
			counter = 1;
		}
		else {
			CKE = 1;
			R[0] = RA[0];
			R[1] = RA[1];
			//R[2] = RA[7);
			R[3] = RA[2];
			R[4] = RA[3];
			R[5] = RA[4];
			counter = 0;
			row_com = Command::NOP;
		}
		break;
	case (int(Command::PRE)):
//		R[3] = BA[4];
		R[4] = 0;
		R[5] = BA[3];
		row_com = Command::NOP;
		break;
	}

	switch (int(col_com)) {
	case (int(Command::NOP)):
		break;
	case (int(Command::RD)):
		CKE = 1;
		C[0] = CA[0];
		C[1] = CA[1];
		C[3] = CA[2];
		C[4] = CA[3];
		C[5] = CA[4];
		C[6] = CA[5];
		//C[7] = BA[4];
		col_com = Command::NOP;
		break;

	case (int(Command::RDA)):
		CKE = 1;
		C[0] = CA[0];
		C[1] = CA[1];
		C[3] = CA[2];
		C[4] = CA[3];
		C[5] = CA[4];
		C[6] = CA[5];
		//C[7] = BA[4];
		col_com = Command::NOP;
		break;
	case (int(Command::WR)):
		CKE = 1;
		C[0] = CA[0];
		C[1] = CA[1];
		C[3] = CA[2];
		C[4] = CA[3];
		C[5] = CA[4];
		C[6] = CA[5];
		//C[7] = BA[4];
		col_com = Command::NOP;
		break;
	case (int(Command::WRA)):
		CKE = 1;
		C[0] = CA[0];
		C[1] = CA[1];
		C[3] = CA[2];
		C[4] = CA[3];
		C[5] = CA[4];
		C[6] = CA[5];
		//C[7] = BA[4];
		col_com = Command::NOP;
		break;
	}
}

void PIN::print(){
	output << CKE;
	for (int i = 0; i < 7; i++) {
		output << R[i];
	}
	for (int i = 0; i < 9; i++) {
		output << C[i];
	}
//	output << endl;
}

void PIN::tick() {
	translate_rising();
	print();
	output << " ";
	translate_falling();
	print();
	output << endl;
}

/*
void PIN::test() {
	input.open("pin.csv");

	input.close;

}*/