#ifndef TESTGAME_H
#define TESTGAME_H

#include <string>

class TestGame : public SearchSpace {

	class State : public SearchState {
	public:
		State();
		State(State& s);
		SearchState* clone();
		virtual void print();
		virtual void expand();
		virtual std::string hash();
		int x, y;
	private:
		int h() const;
		bool goal() const;
	};

public:
	TestGame();
	TestGame(TestGame& s);
	SearchSpace* clone();
	void printInit();
};

void TestGame::State::expand() {
	// Action: Add 2 to x
	State* state1 = new State(*this);
	state1->x += 2;
	state1->g += 1;
	state1->curPath.push_back("X+2");
	state1->f = state1->g + state1->h();
	kids.push(state1);
	// Action: Multiply x by 3
	State* state2 = new State(*this);
	state2->x *= 3;
	state2->g += 1;
	state2->curPath.push_back("X*3");
	state2->f = state2->g + state2->h();
	kids.push(state2);
	// Action: Add 1 to y
	State* state3 = new State(*this);
	state3->y += 1;
	state3->g += 1;
	state3->curPath.push_back("Y+1");
	state3->f = state3->g + state3->h();
	kids.push(state3);
	// Action: Divide y by 2
	State* state4 = new State(*this);
	state4->y /= 2;
	state4->g += 1;
	state4->curPath.push_back("Y/2");
	state4->f = state4->g + state4->h();
	kids.push(state4);
	// Action: Swap x and y
	State* state5 = new State(*this);
	state5->x = y;
	state5->y = x;
	state5->g += 1;
	state5->curPath.push_back("SWAP");
	state5->f = state5->g + state5->h();
	kids.push(state5);
}

void TestGame::State::print() {
	std::cout << x << " " << y << std::endl;
}

int TestGame::State::h() const {
	int total = 0;
	if (x != 999)
		total++;
	if (y != 999)
		total++;
	return total;
}

bool TestGame::State::goal() const {
	return (x == 999 && y == 999);
}

TestGame::State::State(State& s) : SearchState(s) {
	x = s.x;
	y = s.y;
}

std::string TestGame::State::hash() {
	std::string s = "x";
	s += std::to_string(x);
	s += "y";
	s += std::to_string(y);
	return s;
}

TestGame::State::State() {
	std::cout << "Constructor for State" << std::endl;
	x = 0;
	y = 0;
	f = g + h();
}

TestGame::TestGame() {
	std::cout << "Constructor for TestGame" << std::endl;
	init = new State();
}

TestGame::TestGame(TestGame& s) {
	std::cout << "Copy constructor for TestGame" << std::endl;
	State* st = (State*)s.init;
	init = new State(*st);
	std::cout << "Original address of init: " << s.init << std::endl;
	std::cout << "  Cloned address of init: " << init << std::endl;
}

SearchSpace* TestGame::clone() {
	std::cout << "TestGame::clone()" << std::endl;
	return new TestGame(*this);
}

SearchState* TestGame::State::clone() {
	std::cout << "State::clone()" << std::endl;
	return new State(*this);
}

void TestGame::printInit() {
	init->print();
}

#endif TESTGAME_H