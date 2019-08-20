#include "HBM.h"

using namespace std;

HBM::HBM()
{
	init_timing();
}

HBM::~HBM() {
	delete[] node;
	delete timer;
}

int HBM::work(int BA, int RA, int CA, string req, ofstream &out){
	this->BA = BA;
	this->RA = RA;
	this->CA = CA;
	
	int new_command;
	int finish = 0;

	if (req == "R") request = Request::READ;
	else if (req == "W") request = Request::WRITE;
	else out << "wrong request" << endl;

	new_command = change_command((&node[BA])->state, request, RA);
	if (new_command) {
		finish = change_state((&node[BA])->state, (&node[BA])->command, RA);
		out << command_name[int((&node[BA])->command)];
	}
	timer->tick();
	return finish;
}

int HBM::change_state(State state, Command command, int id) {
	switch (int(command)) {
		case (int(Command::ACT)):
			if ((&node[BA])->state == State::Idle) {
				(&node[BA])->state = State::Active;
				(&node[BA])->row_state[id] = State::Active;
			}
			else
				return -1;
			break;
		case (int(Command::PRE)):
			(&node[BA])->state = State::Idle;
			(&node[BA])->row_state.clear();
			break;
		case (int(Command::RD)):
			(&node[BA])->state = State::Reading;
			return 1;
			break;
		case (int(Command::WR)):
			(&node[BA])->state = State::Writing;
			return 1;
			break;
	}
	return 0;
}
int HBM::change_command(State state, Request request, int id) {			//id = RA
	
	//level 계산
	if (BA == prev_BA) level = Level::Bank;
	else if (BA / 4 == prev_BA / 4) level = Level::BankGroup;
	else level = Level::Rank;
	
	switch (int(request)) {
	case (int(Request::READ)):
		switch (int(state)) {
		case (int(State::Idle)):
			if (wait(timer, level, (&node[prev_BA])->command, Command::ACT)) return false;
			(&node[BA])->command = Command::ACT;
			timer->cACT[BA] = 0;
			timer->act.push(timer->time);
			if (timer->act.size() > 4)timer->act.pop();

			break;
		default:	//Active, Reading, Writing, Reading_pre, Writing_pre,
			if ((&node[BA])->row_state[id] == State::Active) {
				if (wait(timer, level, (&node[prev_BA])->command, Command::RD)) return false;
				(&node[BA])->command = Command::RD;
			}
			else {
				if (wait(timer, level, (&node[prev_BA])->command, Command::PRE)) return false;
				(&node[BA])->command = Command::PRE;
			}
		}
		break;
		
	case (int(Request::WRITE)):
		switch (int(state)) {
		case (int(State::Idle)):
			if (wait(timer, level, (&node[prev_BA])->command, Command::ACT)) return false;
			(&node[BA])->command = Command::ACT;
			timer->cACT[BA] = 0;
			timer->act.push(timer->time);
			if (timer->act.size() > 4)timer->act.pop();
			break;
		default:
			if ((&node[BA])->row_state[id] == State::Active) {
				if (wait(timer, level, (&node[prev_BA])->command, Command::WR)) return false;
				(&node[BA])->command = Command::WR;
			}
			else {
				if (wait(timer, level, (&node[prev_BA])->command, Command::PRE)) return false;
				(&node[BA])->command = Command::PRE;
			}
		}
		break;
	}
	prev_BA = BA;
	return true;
}

//level, 이전 command, 다음 command
int HBM::wait(Timer* timer, Level level, Command pre_command, Command command) {
	SpeedEntry& s = speed_table;
	//nFAW
	if (command == Command::ACT) {
		if (timer->act.size() == 4 && timer->time - timer->act.front() < s.nFAW) {
			return true;
		}
	}
	
	//같은 BANK ACT <-> PRE nRAS  ACT <-> ACT nRC
	if (command == Command::ACT) {
		if (timer->cACT[BA] < timing[int(Level::Bank)][int(Command::ACT)][Command::ACT]) {
			return true;
		}
	}
	if (command == Command::PRE) {
		if (timer->cACT[BA] < timing[int(Level::Bank)][int(Command::ACT)][Command::PRE]) {
			return true;
		}
	}
	//이전 command 와의 timing이 0일때
	if (timing[int(level)][int(pre_command)].find(command) == timing[int(level)][int(pre_command)].end()) {
		if (command == Command::ACT) timer->cACT[BA] = 0;
		timer->wait_counter = 0;
		return false;
	}
	//이전 command 와의 timing
	if (timer->wait_counter < timing[int(level)][int(pre_command)][command]) {
		return true;
	}
	else {
		if (command == Command::ACT) timer->cACT[BA] = 0;
		timer->wait_counter = 0;
		return false;
	}
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
	t[int(Command::RD)].insert({ Command::RD, s.nCCDS });
	t[int(Command::RD)].insert({ Command::RDA, s.nCCDS });
	t[int(Command::RDA)].insert({ Command::RD, s.nCCDS });
	t[int(Command::RDA)].insert({ Command::RDA, s.nCCDS });
	t[int(Command::WR)].insert({ Command::WR, s.nCCDS });
	t[int(Command::WR)].insert({ Command::WRA, s.nCCDS });
	t[int(Command::WRA)].insert({ Command::WR, s.nCCDS });
	t[int(Command::WRA)].insert({ Command::WRA, s.nCCDS });
	t[int(Command::RD)].insert({ Command::WR, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::RD)].insert({ Command::WRA, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::RDA)].insert({ Command::WR, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::RDA)].insert({ Command::WRA, s.nCL + s.nCCDS + 2 - s.nCWL });
	t[int(Command::WR)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRS });
	t[int(Command::WR)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRS });
	t[int(Command::WRA)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRS });
	t[int(Command::WRA)].insert({ Command::RDA, s.nCWL + s.nBL + s.nWTRS });

	t[int(Command::RD)].insert({ Command::PREA, s.nRTP });
	t[int(Command::WR)].insert({ Command::PREA, s.nCWL + s.nBL + s.nWR });

	// CAS <-> PD
	t[int(Command::RD)].insert({ Command::PDE, s.nCL + s.nBL + 1 });
	t[int(Command::RDA)].insert({ Command::PDE, s.nCL + s.nBL + 1 });
	t[int(Command::WR)].insert({ Command::PDE, s.nCWL + s.nBL + s.nWR });
	t[int(Command::WRA)].insert({ Command::PDE, s.nCWL + s.nBL + s.nWR + 1 }); // +1 for pre
	t[int(Command::PDX)].insert({ Command::RD, s.nXP });
	t[int(Command::PDX)].insert({ Command::RDA, s.nXP });
	t[int(Command::PDX)].insert({ Command::WR, s.nXP });
	t[int(Command::PDX)].insert({ Command::WRA, s.nXP });

	// CAS <-> SR: none (all banks have to be precharged)

	// RAS <-> RAS
	t[int(Command::ACT)].insert({ Command::ACT, s.nRRDS });
//	t[int(Command::ACT)].insert({ Command::ACT, TimingEntry(4, s.nFAW });
	t[int(Command::ACT)].insert({ Command::PREA, s.nRAS });
	t[int(Command::PREA)].insert({ Command::ACT, s.nRP });

	// RAS <-> REF
	t[int(Command::PRE)].insert({ Command::REF, s.nRP });
	t[int(Command::PREA)].insert({ Command::REF, s.nRP });
	t[int(Command::REF)].insert({ Command::ACT, s.nRFC });

	// RAS <-> PD
	t[int(Command::ACT)].insert({ Command::PDE, 1 });
	t[int(Command::PDX)].insert({ Command::ACT, s.nXP });
	t[int(Command::PDX)].insert({ Command::PRE, s.nXP });
	t[int(Command::PDX)].insert({ Command::PREA, s.nXP });

	// RAS <-> SR
	t[int(Command::PRE)].insert({ Command::SRE, s.nRP });
	t[int(Command::PREA)].insert({ Command::SRE, s.nRP });
	t[int(Command::SRX)].insert({ Command::ACT, s.nXS });

	// REF <-> REF
	t[int(Command::REF)].insert({ Command::REF, s.nRFC });

	// REF <-> PD
	t[int(Command::REF)].insert({ Command::PDE, 1 });
	t[int(Command::PDX)].insert({ Command::REF, s.nXP });

	// REF <-> SR
	t[int(Command::SRX)].insert({ Command::REF, s.nXS });

	// PD <-> PD
	t[int(Command::PDE)].insert({ Command::PDX, s.nPD });
	t[int(Command::PDX)].insert({ Command::PDE, s.nXP });

	// PD <-> SR
	t[int(Command::PDX)].insert({ Command::SRE, s.nXP });
	t[int(Command::SRX)].insert({ Command::PDE, s.nXS });

	// SR <-> SR
	t[int(Command::SRE)].insert({ Command::SRX, s.nCKESR });
	t[int(Command::SRX)].insert({ Command::SRE, s.nXS });

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
	t[int(Command::WR)].insert({ Command::WR, s.nCCDL });
	t[int(Command::WR)].insert({ Command::WRA, s.nCCDL });
	t[int(Command::WRA)].insert({ Command::WR, s.nCCDL });
	t[int(Command::WRA)].insert({ Command::WRA, s.nCCDL });
	t[int(Command::WR)].insert({ Command::RD, s.nCWL + s.nBL + s.nWTRL });
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
	t[int(Command::WRA)].insert({ Command::ACT, s.nCWL + s.nBL + s.nWR + s.nRP });

	// RAS <-> RAS
	t[int(Command::ACT)].insert({ Command::ACT, s.nRC });
	t[int(Command::ACT)].insert({ Command::PRE, s.nRAS });
	t[int(Command::PRE)].insert({ Command::ACT, s.nRP });

	// REFSB
	t[int(Command::PRE)].insert({ Command::REFSB, s.nRP });
	t[int(Command::REFSB)].insert({ Command::REFSB, s.nRFC });
	t[int(Command::REFSB)].insert({ Command::ACT, s.nRFC });
}
