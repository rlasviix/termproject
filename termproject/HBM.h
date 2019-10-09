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
#include "pin.h"

using namespace std;

#define num_bank 16

class HBM
{
public:
	HBM();
	~HBM();

	
	enum class Request : int
	{
		READ, WRITE, PRECHARGE, REFRESH,
		MAX
	};

	/* Command */
	enum class Command : int
	{
		ACT, PRE, PREA,
		RD, WR, RDA, WRA,
		REF, REFSB, PDE, PDX, SRE, SRX,
		NOP,
		MAX
	};
	string command_name[int(Command::MAX)] = {
		"ACT", "PRE",   "PREA",
		"RD",  "WR",    "RDA",  "WRA",
		"REF", "REFSB", "PDE",  "PDX",  "SRE", "SRX"
		"NOP"
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
	map<Command, int> timing[int(Level::MAX)][int(Command::MAX)];
	
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
	} speed_table = {
		1000, 
		500, 2.0, 
		2, 2, 3, 
		7, 7, 6, 7, 4, 
		17, 24, 
		7, 2, 4, 8, 
		4, 5, 20,
		0, 1950, 0, 
		5, 5, 
		5, 0}
	, speed_entry;
	
	Node<HBM>* node = new Node<HBM>[num_bank];	

	Timer* timer = new Timer();
	PIN* pin = new PIN();

	int BA, RA, CA;
	bool pre;
	Request request;
	bool work(bool pre, int BA, int RA, int CA, string command, ofstream& out);

	

private:
	bool change_state(State state, Command command, int ra);
	bool change_command(State state, Request request, int ra);
	bool wait(int bank, Command command);
	Level calculate_level(int cur_bank, int prev_bank);
	void init_timing();
};