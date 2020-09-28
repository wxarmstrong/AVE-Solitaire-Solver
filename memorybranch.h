#ifndef MEMORYBRANCH_H
#define MEMORYBRANCH_H

#include "search.h"
#include <map>
#include <string>

struct MemoryBranch : public SearchAlg {
	MemoryBranch(SearchDomain* d);
	virtual Solution solve();
	virtual void solve(SearchState* s);
	std::map<std::string, int> StateList;
};

MemoryBranch::MemoryBranch(SearchDomain* d) {
	dom = d;
	best.cost = 4000;
}

Solution MemoryBranch::solve() {
	solve(dom->init);
	return best;
}

void MemoryBranch::solve(SearchState* s) {
	/*
	std::cout << std::endl;
	std::cout << "Current state:" << std::endl;
	std::cout << "f = " << s->f << ", g = " << s->g << ", h = " << s->h() << std::endl;
	*/
	//s->curPath.print();
	//system("pause");

	s->curPath.cost = s->g;
	// If goal met, set as new best path
	if (s->goal())
	{
		best = s->curPath;
		std::cout << "New best path found: " << std::endl;
		best.print();
		std::cout << std::endl;
		//system("pause");
		return;
	}

	// Check if in state table
	std::string curHash = s->hash();
	int count = StateList.count(curHash);
	if (count > 0) {
		int prevF = StateList[curHash];
		if (s->f >= prevF)
			return;
	}

	if (StateList.size() > 100000) {
		//std::cout << "StateList too big! Deleting arbitrary elements:" << std::endl;
		while (StateList.size() > 100000)
			StateList.erase(StateList.begin());
	}

	StateList[curHash] = s->f;

	// Populate the state's kids queue
	s->expand();

	// The list is full of every kid but we need to trim the unviable ones first
	std::priority_queue<SearchState*, std::vector<SearchState*>, SearchState::Compare> tempKids;

	while (!s->kids.empty()) {
		SearchState* nextState = s->kids.top();
		s->kids.pop();
		if (nextState->f < best.cost)
		{
			std::string hash = nextState->hash();
			int count = StateList.count(hash);
			if (count == 0)
				tempKids.push(nextState);
			else {
				if (StateList[hash] > nextState->f) {
					StateList[hash] = nextState->f;
					tempKids.push(nextState);
				}
				else {
					delete nextState;
				}
			}
		}
		else
			delete nextState;
	}

	s->kids = tempKids;
	while (!s->kids.empty()) {
		SearchState* nextState = s->kids.top();
		s->kids.pop();
		if (nextState->f < best.cost)
			solve(nextState);
		delete nextState;
	}
}


#endif MEMORYBRANCH_H