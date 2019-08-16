#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <map>
#include <functional>
#include <algorithm>
#include <cassert>
#include <type_traits>

#include "node.h"
#include "timer.h"

using namespace std;

class HBM
{
public:
	HBM();
	~HBM();
	enum class Request : int
	{
		READ, WRITE,
		MAX
	};
	/* Command */
	enum class Command : int
	{
		ACT, PRE, PREA,
		RD, WR, RDA, WRA,
		REF, REFSB, PDE, PDX, SRE, SRX,
		MAX
	};
	string command_name[int(Command::MAX)] = {
		"ACT", "PRE",   "PREA",
		"RD",  "WR",    "RDA",  "WRA",
		"REF", "REFSB", "PDE",  "PDX",  "SRE", "SRX"
	};

	/* Level */
	enum class Level : int
	{
		Channel, Rank, BankGroup, Bank, Row, Column, MAX
	};
	
	/* State */
	enum class State : int
	{
		Idle, Active, 
		Reading, Writing, Reading_pre, Writing_pre, 
		Pre_charging , MAX
	};

	/* Timing */
	struct TimingEntry
	{
		TimingEntry() :dist(0), val(0) {}
		TimingEntry(int distance, int value) : dist(distance), val(value) {}
//		Command cmd;
		int dist;
		int val;
	//	bool sibling;
	};
//	vector<TimingEntry> timing[int(Level::MAX)][int(Command::MAX)];
	map<Command, TimingEntry> timing[int(Level::MAX)][int(Command::MAX)];
	


	struct SpeedEntry {
		int rate;
		double freq, tCK;
		int nBL, nCCDS, nCCDL;
		int nCL, nRCDR, nRCDW, nRP, nCWL;
		int nRAS, nRC;
		int nRTP, nWTRS, nWTRL, nWR;
		int nRRDS, nRRDL, nFAW;
		int nRFC, nREFI, nREFI1B;
		int nPD, nXP;
		int nCKESR, nXS;
	} speed_table = {1000, 500, 2.0, 2, 2, 3, 7, 7, 6, 7, 4, 17, 24, 7, 2, 4, 8, 4, 5, 20, 0, 1950, 0, 5, 5, 5, 0}
	, speed_entry;
	



	Node<HBM>* node = new Node<HBM>[16];		//16 banks

	Timer* timer = new Timer();

	Level level;
	int prev_BA;
//	int SID;
	int BA, RA, CA;
	int counter = 0;
	Request request;
	
//	int work(int SID, int BA, int RA, int CA, string command, ofstream &out);
	int work(int BA, int RA, int CA, string command, ofstream& out);


private:
	int change_state(State state, Command command, int id);
	int change_command(State state, Request request, int id);

	int wait(Timer* timer, Level level, Command pre_command, Command command);
//	int data(Command command);

	void init_timing();
};