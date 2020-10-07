#ifndef IDASTAR_H
#define IDASTAR_H

#include "search.h"

class IDAStar : public SearchAlg {
public:
	IDAStar(SearchSpace& s);
	virtual Solution solve();
private:
	int search(SearchState* s, int threshold);
};

IDAStar::IDAStar(SearchSpace& s) {
//	std::cout << "Constructing IDAStar with SearchSpace s" << std::endl;
	dom = s.clone();
}

Solution IDAStar::solve() {
	SearchState* init = dom->init;
	int threshold = init->h();
	while (true) {
		std::cout << "NEW ITERATION: Threshold is " << threshold << std::endl;
		int temp = search(init, threshold);
		if (temp == 0)
			return best;
		threshold = temp;
	}
}

int IDAStar::search(SearchState* s, int threshold) {
	if (s->f > threshold)
		return s->f;

	if (s->goal())
	{
		best = s->curPath;
		best.cost = s->g;
		return 0;
	}

	int min = INT_MAX;
	s->expand();
	while (!(s->kids).empty())
	{
		SearchState* nextNode = s->kids.top();
		s->kids.pop();
		int temp = search(nextNode, threshold);
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
	return min;
}

#endif IDASTAR_H