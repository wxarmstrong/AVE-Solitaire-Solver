#ifndef BRANCH_H
#define BRANCH_H

#include "search.h"
#include <map>

class Branch : public SearchAlg {
public:
	Branch(SearchSpace& s);
	virtual Solution solve();
	void solve(SearchState* s);
	std::map<std::string, int> StateList;
};

Branch::Branch(SearchSpace& s) {
	std::cout << "Constructing Branch with SearchSpace s" << std::endl;
	dom = s.clone();
	best.cost = 100;
}

Solution Branch::solve() {
	solve(dom->init);
	return best;
}

void Branch::solve(SearchState* s) {
	
	//std::cout << std::endl;
	//std::cout << "Current state:" << std::endl;
	//std::cout << "f = " << s->f << ", g = " << s->g << ", h = " << s->h() << std::endl;
	
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

#endif BRANCH_H