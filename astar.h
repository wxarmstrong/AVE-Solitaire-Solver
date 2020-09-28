#ifndef ASTAR_H
#define ASTAR_H

#define PQ std::priority_queue<SearchState*, std::vector<SearchState*>, SearchState::Compare>

#include "search.h"
#include <map>
#include <queue>

struct AStar : public SearchAlg {
	AStar(SearchDomain* d);
	virtual Solution solve();
	std::map<std::string, int> StateList;
};

AStar::AStar(SearchDomain* d) {
	dom = d;
}

Solution AStar::solve() {
	PQ fringe;

	fringe.push(dom->init);
	while ( !fringe.empty() )
	{
		SearchState* s = fringe.top();
		fringe.pop();
		std::cout << "Testing state: f = " << s->f << std::endl;

		if (s->goal())
		{
			best = s->curPath;
			std::cout << "Optimal path found: " << std::endl;
			best.print();
			std::cout << std::endl;
			return best;
		}
		
		std::string curHash = s->hash();
		int count = StateList.count(curHash);
		if (count == 0)
		{
			StateList[curHash] = s->f;
			s->expand();
			while (!s->kids.empty())
			{
				SearchState* k = s->kids.top();
				s->kids.pop();
				fringe.push(k);
			}
		}
		delete s;
	}

	return best;
}



#endif ASTAR_H