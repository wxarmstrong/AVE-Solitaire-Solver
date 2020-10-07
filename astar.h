#ifndef ASTAR_H
#define ASTAR_H

#include <map>
#include "search.h"

class AStar : public SearchAlg {
public:
	AStar(SearchSpace& s);
	virtual Solution solve();
	std::map<std::string, int> StateList;
};

AStar::AStar(SearchSpace& s) {
	std::cout << "Constructing AStar with SearchSpace s" << std::endl;
	dom = s.clone();
}

Solution AStar::solve() {
	PQ fringe;
	SearchState* in = dom->init->clone();
	StateList[in->hash()] = in->f;
	fringe.push(in);

	while (!fringe.empty())
	{
		SearchState* s = fringe.top();
		fringe.pop();
		//std::cout << "Testing state: f = " << s->f << std::endl;

		if (s->goal())
		{
			best = s->curPath;
			best.cost = s->g;
			std::cout << "Optimal path found: " << std::endl;
			best.print();
			std::cout << std::endl;

			// Clear PQ
			delete s;
			while (!fringe.empty())
			{
				s = fringe.top();
				fringe.pop();
				delete s;
			}

			return best;
		}

		s->expand();
		while (!s->kids.empty())
		{
			SearchState* k = s->kids.top();
			std::string hash = k->hash();
			int count = StateList.count(hash);
			if (count == 0 || k->f < StateList[hash])
			{
				StateList[hash] = k->f;
				fringe.push(k);
			}
			else
			{
				delete k;
			}
			s->kids.pop();
		}

		delete s;
	}

	return best;
}

#endif ASTAR_H