#include <fstream>
#include <iostream>

class SearchState {

};

class SearchSpace {
public:
	virtual ~SearchSpace() { 
		std::cout << "Base destruction (SearchSpace)" << std::endl; 
		delete init;
	}
protected:
	SearchState* init = nullptr;
};

class SearchAlg {
public:
	SearchAlg(SearchSpace* s) {
		// dom = deep copy of s
	}
	virtual ~SearchAlg() {
		std::cout << "Base destruction (SearchAlg)" << std::endl;
		delete dom;
	}
protected:
	SearchSpace* dom;
};

class AStar : public SearchAlg {
public:
	AStar(SearchSpace* s) {
		// dom = deep copy of s
	}
	virtual ~AStar() {
		std::cout << "Derived destruction (AStar)" << std::endl;
	}
};

class Solitaire : public SearchSpace {

	class State : public SearchState {
	public:
		State(std::ifstream& in);
	};

public:
	Solitaire(std::ifstream& in);
	virtual ~Solitaire() { std::cout << "Derived destruction (Solitaire)" << std::endl; }
};

Solitaire::Solitaire(std::ifstream& in) {
	init = new State(in);
}

Solitaire::State::State(std::ifstream& in) {
	// Create initial state from instance file
}

int main() {
	while (true) {
		std::ifstream in("instance.txt");
		Solitaire* s = new Solitaire(in);
		AStar* solver = new AStar(s);
		delete solver;
		delete s;
		in.close();
		std::cout << std::endl;
	}
	return 0;
}