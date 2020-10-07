#include <fstream>
#include "idastar_m.h"
#include "solitaire.h"
#include "testgame.h"

int main()
{
	std::ifstream in("instance.txt");
	//Solitaire s(in);
	TestGame s;
	s.printInit();
	IDAStarM solver(s);
	Solution sol = solver.solve();
	sol.print();
}