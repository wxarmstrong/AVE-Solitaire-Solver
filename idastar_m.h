#ifndef IDASTAR_M_H
#define IDASTAR_M_H

#include "idastar.h"
#include <map>

class IDAStarM : public SearchAlg {
public:
	IDAStarM(SearchSpace& s);
	virtual Solution solve();
	std::map<std::string, int> StateList;
private:
	int search(SearchState* s, int threshold);
};

IDAStarM::IDAStarM(SearchSpace& s) {
	//	std::cout << "Constructing IDAStar with SearchSpace s" << std::endl;
	dom = s.clone();
}

Solution IDAStarM::solve() {
	SearchState* init = dom->init;
	int threshold = init->h();
	//int threshold = 1100;
	while (true) {
		StateList.clear();
		StateList[init->hash()] = init->g;
		std::cout << "NEW ITERATION: Threshold is " << threshold << std::endl;
		int temp = search(init, threshold);
		if (temp == 0)
			return best;
		threshold = temp;
	}
}

int IDAStarM::search(SearchState* s, int threshold) {
//	s->curPath.print();
	if (s->f > threshold)
		return s->f;

	if (s->goal())
	{
		best = s->curPath;
		best.cost = s->g;
		return 0;
	}

	StateList[s->hash()] = INT_MAX;
	int min = INT_MAX;
	s->expand();
	while (!(s->kids).empty())
	{
		SearchState* nextNode = s->kids.top();
		s->kids.pop();
		int temp;
		std::string hash = nextNode->hash();
		if (StateList.count(hash) == 0 || StateList[hash] > nextNode->f)
		{
			temp = search(nextNode, threshold);
		}
		else {
			temp = StateList[hash];
		}
		delete nextNode;
		if (temp == 0)
		{
			while (!(s->kids).empty())
			{
				nextNode = s->kids.top();
				s->kids.pop();
				delete nextNode;
			}
			return 0;
		}
		if (temp < min)
			min = temp;
	}
	StateList[s->hash()] = min;
	return min;
}

#endif IDASTAR_M_H