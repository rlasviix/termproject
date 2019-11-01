#include "HBM.h"

using namespace std;

HBM::HBM()
{
	init_timing();
}

HBM::~HBM() {
	delete[] node;
	delete timer;
	delete pin;
}

bool HBM::work(bool pre, int BA, int RA, int CA, string req, ofstream &out){
	this->BA = BA;
	this->RA = RA;
	this->CA = CA;
	this->pre = pre;

	int new_command;
	bool finish = false;

	if (req == "NOP") {
		pin->tick();
		timer->tick();
		return true;
	}

	if (req == "R") request = Request::READ;
	else if (req == "W") request = Request::WRITE;
	else if (req == "PRE") request = Request::PRECHARGE;
	else if (req == "REF") request = Request::REFRESH;
	else out << "wrong request" << endl;

	new_command = change_command(node[BA].state, request, RA);
	if (new_command) {
		finish = change_state(node[BA].state, node[BA].command, RA);
		out.width(3);
		out << timer->time << "   BANK = ";
		out.width(3);
		out << BA << "   RA = ";
		out.width(3);
		out << RA << "   ";
		out << command_name[int(node[BA].command)];
		out << endl;
		pin->set_command(int(node[BA].command), BA, RA, CA);
	}
	
	pin->tick();
	timer->tick();
	return finish;
}

bool HBM::change_state(State state, Command command, int ra) {
	switch (int(command)) {
		case (int(Command::ACT)):
			if (node[BA].state == State::Idle) {
				node[BA].state = State::Active;
				node[BA].row_state[ra] = State::Active;
			}
			else
				return false;
			break;
		case (int(Command::PRE)):
			node[BA].state = State::Idle;
			node[BA].row_state.clear();
			if (request == Request::PRECHARGE) return true;
			else return false;
		case (int(Command::PREA)):
			for (int i = 0; i < num_bank; i++) {
				node[i].state = State::Idle;
				node[i].row_state.clear();
			}
			return false;
		case (int(Command::RD)):
			node[BA].state = State::Reading;
			if (request == Request::READ) return true;
			else return false;
		case (int(Command::WR)):
			node[BA].state = State::Writing;
			if (request == Request::WRITE) return true;
			else return false;
		case (int(Command::RDA)):
			node[BA].state = State::Idle;
			node[BA].row_state.clear();
			if (request == Request::READ) return true;
			else return false;
		case (int(Command::WRA)):
			node[BA].state = State::Idle;
			node[BA].row_state.clear();
			if (request == Request::WRITE) return true;
			else return false;
		case (int(Command::REF)):
			node[BA].state = State::Idle;
			if (request == Request::REFRESH) return true;
			else return false;
	}
	return 0;
}

bool HBM::change_command(State state, Request request, int ra) {
	switch (int(request)) {
	case (int(Request::READ)):
		switch (int(state)) {
		case (int(State::Idle)):
			if (wait(BA, Command::ACT)) return false;
			node[BA].command = Command::ACT;
			break;
		default:	//Active, Reading, Writing
			if (node[BA].row_state[ra] == State::Active) {
				//RD 쓸지 RDA 쓸지
				if (pre) {
					if (wait(BA, Command::RDA)) return false;
					node[BA].command = Command::RDA;
				}
				else {
					if (wait(BA, Command::RD)) return false;
					node[BA].command = Command::RD;
				}
			}
			else {
				if (wait(BA, Command::PRE)) return false;
				node[BA].command = Command::PRE;
			}
		}
		break;
		
	case (int(Request::WRITE)):
		switch (int(state)) {
		case (int(State::Idle)):
			if (wait(BA, Command::ACT)) return false;
			node[BA].command = Command::ACT;
			break;
		default:
			if (node[BA].row_state[ra] == State::Active) {
				if (pre) {
					if (wait(BA, Command::WRA)) return false;
					node[BA].command = Command::WRA;
				}
				else {
					if (wait(BA, Command::WR)) return false;
					node[BA].command = Command::WR;
				}
			}
			else {
				if (wait(BA, Command::PRE)) return false;
				node[BA].command = Command::PRE;
			}
		}
		break;

	case(int(Request::PRECHARGE)):
		if (wait(BA, Command::PRE)) return false;
		node[BA].command = Command::PRE;
	//////////////////////////////////////////////////////////////////////////////////////////
	case (int(Request::REFRESH)):
		//모든 bank idle 인지 확인
		all_idle = true;
		for (int i = 0; i < num_bank; i++) {
			if (node[i].state != State::Idle) {
				all_idle = false;
				break;
			}
		}

		//모든 BANK IDLE - REF
		if (all_idle) {
			for (int i = 0; i < num_bank; i++) {
				if (wait(i, Command::REF)) return false;
			}
			for (int i = 0; i < num_bank; i++) {
				node[i].command = Command::REF;
			}
			break;
		}
		//ACT BANK 존재 - PREA
		else {
			for (int i = 0; i < num_bank; i++) {
				if (wait(i, Command::PREA)) return false;
			}
			for (int i = 0; i < num_bank; i++) {
				node[BA].command = Command::PREA;
			}
		}
		break;
	}
	


	return true;
}

bool HBM::wait(int bank, Command command) {
	switch (command) {
	case(Command::ACT):
		if (node[bank].next_activate > timer->time) return true;

		//FAW
		timer->act.push(timer->time);
		if (timer->act.size() > 4) timer->act.pop();

		//next timing 계산
		for (int i = 0; i < num_bank; i++) {
			int temp_level = int(calculate_level(i, bank));
			node[i].next_activate = max(node[i].next_activate, timer->time + timing[temp_level][int(Command::ACT)][Command::ACT]);
			if(timer->act.size() == 4) node[i].next_activate = max(node[i].next_activate, timer->act.front() + speed_table.nFAW);
		}
		node[bank].next_read = max(node[bank].next_read, timer->time + timing[int(Level::Bank)][int(Command::ACT)][Command::RD]);
		node[bank].next_write = max(node[bank].next_write, timer->time + timing[int(Level::Bank)][int(Command::ACT)][Command::WR]);
		node[bank].next_RDA = max(node[bank].next_RDA, timer->time + timing[int(Level::Bank)][int(Command::ACT)][Command::RDA]);
		node[bank].next_WRA = max(node[bank].next_WRA, timer->time + timing[int(Level::Bank)][int(Command::ACT)][Command::WRA]);
		node[bank].next_precharge = max(node[bank].next_precharge, timer->time + timing[int(Level::Bank)][int(Command::ACT)][Command::PRE]);
		break;
	case(Command::RD):
		if (node[bank].next_read > timer->time) return true;
		for (int i = 0; i < num_bank; i++) {
			int temp_level = int(calculate_level(i, bank));
			node[i].next_read = max(node[i].next_read, timer->time + timing[temp_level][int(Command::RD)][Command::RD]);
			node[i].next_RDA = max(node[i].next_RDA, timer->time + timing[temp_level][int(Command::RD)][Command::RDA]);
			if (timing[int(temp_level)][int(Command::RD)].find(Command::WR) != timing[int(temp_level)][int(Command::RD)].end()) {
				node[i].next_write = max(node[i].next_write, timer->time + timing[temp_level][int(Command::RD)][Command::WR]);
				node[i].next_WRA = max(node[i].next_WRA, timer->time + timing[temp_level][int(Command::RD)][Command::WRA]);
			}
		}
		node[bank].next_precharge = max(node[bank].next_precharge, timer->time + timing[int(Level::Bank)][int(Command::RD)][Command::WR]);
		break;
	case(Command::WR):
		if (node[bank].next_write > timer->time) return true;
		for (int i = 0; i < num_bank; i++) {
			int temp_level = int(calculate_level(i, bank));
			node[i].next_read = max(node[i].next_read, timer->time + timing[temp_level][int(Command::WR)][Command::RD]);
			node[i].next_RDA = max(node[i].next_RDA, timer->time + timing[temp_level][int(Command::WR)][Command::RDA]);
			if (timing[int(temp_level)][int(Command::WR)].find(Command::WR) != timing[int(temp_level)][int(Command::WR)].end()) {
				node[i].next_write = max(node[i].next_write, timer->time + timing[temp_level][int(Command::WR)][Command::WR]);
				node[i].next_WRA = max(node[i].next_WRA, timer->time + timing[temp_level][int(Command::WR)][Command::WRA]);
			}
		}
		node[bank].next_precharge = max(node[bank].next_precharge, timer->time + timing[int(Level::Bank)][int(Command::WR)][Command::WR]);
		break;

	case(Command::RDA):
		if (node[bank].next_read > timer->time) return true;
		for (int i = 0; i < num_bank; i++) {
			int temp_level = int(calculate_level(i, bank));
			node[i].next_read = max(node[i].next_read, timer->time + timing[temp_level][int(Command::RDA)][Command::RD]);
			node[i].next_RDA = max(node[i].next_RDA, timer->time + timing[temp_level][int(Command::RDA)][Command::RDA]);
			if (timing[int(temp_level)][int(Command::RDA)].find(Command::WR) != timing[int(temp_level)][int(Command::RDA)].end()) {
				node[i].next_write = max(node[i].next_write, timer->time + timing[temp_level][int(Command::RDA)][Command::WR]);
				node[i].next_WRA = max(node[i].next_WRA, timer->time + timing[temp_level][int(Command::RDA)][Command::WRA]);
			}
		}
		node[bank].next_activate = max(node[bank].next_activate, timer->time + timing[int(Level::Bank)][int(Command::RDA)][Command::ACT]);
		break;
	case(Command::WRA):
		if (node[bank].next_write > timer->time) return true;
		for (int i = 0; i < num_bank; i++) {
			int temp_level = int(calculate_level(i, bank));
			node[i].next_read = max(node[i].next_read, timer->time + timing[temp_level][int(Command::WRA)][Command::RD]);
			node[i].next_RDA = max(node[i].next_RDA, timer->time + timing[temp_level][int(Command::WRA)][Command::RDA]);
			if (timing[int(temp_level)][int(Command::WRA)].find(Command::WR) != timing[int(temp_level)][int(Command::WRA)].end()) {
				node[i].next_write = max(node[i].next_write, timer->time + timing[temp_level][int(Command::WRA)][Command::WR]);
				node[i].next_WRA = max(node[i].next_WRA, timer->time + timing[temp_level][int(Command::WRA)][Command::WRA]);
			}
		}
		node[bank].next_activate = max(node[bank].next_activate, timer->time + timing[int(Level::Bank)][int(Command::WRA)][Command::ACT]);
		break;

	case(Command::PRE):
		if (node[bank].next_precharge > timer->time) return true;
		node[bank].next_activate = max(node[bank].next_activate, timer->time + timing[int(Level::Bank)][int(Command::PRE)][Command::ACT]);
		node[bank].next_refresh = max(node[bank].next_refresh, timer->time + timing[int(Level::Rank)][int(Command::PRE)][Command::REF]);
		break;
	case(Command::PREA):
		if (node[bank].next_precharge > timer->time) return true;
		node[bank].next_activate = max(node[bank].next_activate, timer->time + timing[int(Level::Bank)][int(Command::PRE)][Command::ACT]);
		node[bank].next_refresh = max(node[bank].next_refresh, timer->time + timing[int(Level::Rank)][int(Command::PRE)][Command::REF]);
		break;
	case(Command::REF):
		if (node[bank].next_refresh > timer->time) return true;
		node[bank].next_activate = max(node[bank].next_activate, timer->time + timing[int(Level::Rank)][int(Command::REF)][Command::ACT]);
		node[bank].next_refresh = max(node[bank].next_refresh, timer->time + timing[int(Level::Rank)][int(Command::REF)][Command::REF]);
		break;
	}
	return false;
}

HBM::Level HBM::calculate_level(int cur_bank, int prev_bank) {
	if (cur_bank == prev_bank) return Level::Bank;
	else if (cur_bank >> 2 == prev_bank >> 2) return Level::BankGroup;
	else return Level::Rank;
}

void HBM::init_timing()
{
	SpeedEntry& s = speed_table;
	map<Command, int>* t;

	/*** Channel ***/												//same channel
	t = timing[int(Level::Channel)];

	// CAS <-> CAS
	t[int(Command::RD)].insert({ Command::RD, s.nBL });
	t[int(Command::RD)].insert({ Command::RDA, s.nBL });
	t[int(Command::RDA)].insert({ Command::RD, s.nBL });
	t[int(Command::RDA)].insert({ Command::RDA, s.nBL });
	t[int(Command::WR)].insert({ Command::WR, s.nBL });
	t[int(Command::WR)].insert({ Command::WRA, s.nBL });
	t[int(Command::WRA)].insert({ Command::WR, s.nBL });
	t[int(Command::WRA)].insert({ Command::WRA, s.nBL });

	
	/*** Rank ***/													//different bank group
	t = timing[int(Level::Rank)];

	// CAS <-> CAS
	t[int(Command::RD)].insert({ Command::RD, s.nCCDS });						//2
	t[int(Command::RD)].insert({ Command::RDA, s.nCCDS });
	t[int(Command::RDA)].insert({ Command::RD, s.nCCDS });
	t[int(Command::RDA)].insert({ Command::RDA, s.nCCDS });
	t[int(Command::WR)].insert({ Command::WR, s.nCCDS });
	t[int(Command::WR)].insert({ Command::WRA, s.nCCDS });
	t[int(Command::WRA)].insert({ Command::WR, s.nCCDS });
	t[int(Command::WRA)].insert({ Command::WRA, s.nCCDS });
	t[int(Command::RD)].insert({ Command::WR, s.nCL + s.nCCDS + 2 - s.nCWL });	//7
	t[int(Command::RD)].insert({ Command::WRA, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::RDA)].insert({ Command::WR, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::RDA)].insert({ Command::WRA, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::WR)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRS });		//8
	t[int(Command::WR)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRS });
	t[int(Command::WRA)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRS });
	t[int(Command::WRA)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRS });

	t[int(Command::RD)].insert({ Command::PREA, s.nRTP });
	t[int(Command::WR)].insert({ Command::PREA, s.nCWL + s.nBL + s.nWR });		//14

	// RAS <-> RAS
	t[int(Command::ACT)].insert({ Command::ACT, s.nRRDS });
	t[int(Command::ACT)].insert({ Command::PREA, s.nRAS });
	t[int(Command::PREA)].insert({ Command::ACT, s.nRP });

	// RAS <-> REF
	t[int(Command::PRE)].insert({ Command::REF, s.nRP });
	t[int(Command::PREA)].insert({ Command::REF, s.nRP });
	t[int(Command::REF)].insert({ Command::ACT, s.nRFC });

	// REF <-> REF
	t[int(Command::REF)].insert({ Command::REF, s.nRFC });


	/*** Bank Group ***/											//same bank group
	t = timing[int(Level::BankGroup)];
	// CAS <-> CAS
	t[int(Command::RD)].insert({ Command::RD, s.nCCDL });
	t[int(Command::RD)].insert({ Command::RDA, s.nCCDL });
	t[int(Command::RDA)].insert({ Command::RD, s.nCCDL });
	t[int(Command::RDA)].insert({ Command::RDA, s.nCCDL });
	t[int(Command::WR)].insert({ Command::WR, s.nCCDL });
	t[int(Command::WR)].insert({ Command::WRA, s.nCCDL });
	t[int(Command::WRA)].insert({ Command::WR, s.nCCDL });
	t[int(Command::WRA)].insert({ Command::WRA, s.nCCDL });
	t[int(Command::WR)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRL });	//10
	t[int(Command::WR)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRL });
	t[int(Command::WRA)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRL });
	t[int(Command::WRA)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRL });

	// RAS <-> RAS
	t[int(Command::ACT)].insert({ Command::ACT, s.nRRDL });

	/*** Bank ***/													//same bank
	t = timing[int(Level::Bank)];

	// CAS <-> RAS
	t[int(Command::ACT)].insert({ Command::RD, s.nRCDR });
	t[int(Command::ACT)].insert({ Command::RDA, s.nRCDR });
	t[int(Command::ACT)].insert({ Command::WR, s.nRCDW });
	t[int(Command::ACT)].insert({ Command::WRA, s.nRCDW });

	t[int(Command::RD)].insert({ Command::PRE, s.nRTP });
	t[int(Command::WR)].insert({ Command::PRE, s.nCWL + s.nBL + s.nWR });

	t[int(Command::RDA)].insert({ Command::ACT, s.nRTP + s.nRP });
	t[int(Command::WRA)].insert({ Command::ACT, s.nCWL + s.nBL + s.nWR + s.nRP });	//21

	// RAS <-> RAS
	t[int(Command::ACT)].insert({ Command::ACT, s.nRC });
	t[int(Command::ACT)].insert({ Command::PRE, s.nRAS });
	t[int(Command::PRE)].insert({ Command::ACT, s.nRP });

	// REFSB
	t[int(Command::PRE)].insert({ Command::REFSB, s.nRP });
	t[int(Command::REFSB)].insert({ Command::REFSB, s.nRFC });
	t[int(Command::REFSB)].insert({ Command::ACT, s.nRFC });

	//임시로 추가
	t[int(Command::RD)].insert({ Command::RD, 0 });
	t[int(Command::RD)].insert({ Command::RDA, 0 });
	t[int(Command::RDA)].insert({ Command::RD, 0 });
	t[int(Command::RDA)].insert({ Command::RDA, 0 });
	t[int(Command::WR)].insert({ Command::WR, 0 });
	t[int(Command::WR)].insert({ Command::WRA, 0 });
	t[int(Command::WRA)].insert({ Command::WR, 0 });
	t[int(Command::WRA)].insert({ Command::WRA, 0 });
	t[int(Command::WR)].insert({ Command::WR, 0 });
	t[int(Command::WR)].insert({ Command::WRA, 0 });
	t[int(Command::WRA)].insert({ Command::WR, 0 });
	t[int(Command::WRA)].insert({ Command::WRA, 0 });
	t[int(Command::WR)].insert({ Command::RD, 0 });
	t[int(Command::WR)].insert({ Command::RDA, 0 });
	t[int(Command::WRA)].insert({ Command::RD, 0 });
	t[int(Command::WRA)].insert({ Command::RDA, 0 });
}
