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

class PIN
{
public:
	PIN();
	~PIN();
	
	enum class Command : int
	{
		ACT, PRE, PREA,
		RD, WR, RDA, WRA,
		REF, REFSB, PDE, PDX, SRE, SRX,
		NOP,
		MAX
	};


	void set_command(int command, int bank_add, int row_add, int col_add);
	void tick();

	ofstream output;
	ifstream input;
	Command row_com;
	Command col_com;
	bool BA[4], RA[14], CA[6];
	int counter = 0;
	bool CKE = 0;
	bool R[7] = { 0, };
	bool C[9] = { 0, };
private:
	void translate_rising();
	void translate_falling();
	void print();
};