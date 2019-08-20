#pragma once

using namespace std;

template<typename T>
class Node {
public:
	Node() :state(T::State::Idle), command(T::Command::MAX) {}
	typename T::State state;
	typename T::Command command;
	map<int, typename T::State> row_state;

};