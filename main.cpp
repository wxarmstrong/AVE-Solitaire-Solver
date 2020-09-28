#include <iostream>
#include <fstream>
#include "memorybranch.h"
#include "solitaire.h"

int main() {
	std::ifstream in("instance.txt");
	Solitaire* s = new Solitaire(in);
	s->printInit();
	MemoryBranch* solver = new MemoryBranch(s);
	Solution sol = solver->solve();
	if (!sol.empty())
		sol.print();
	else
		std::cout << "No solution found" << std::endl;
	return 0;
}