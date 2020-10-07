#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <iostream>
#include <queue>

#define PQ std::priority_queue<SearchState*, std::vector<SearchState*>, SearchState::Compare>

class Solution : public std::vector<std::string> {
public:
	void print();
	int cost = 0;
};

void Solution::print() {
	std::cout << "Cost: " << cost << std::endl;
	for (int i = 0; i < size(); i++)
		std::cout << at(i) << " ";
	std::cout << std::endl;
}

struct SearchState {

	struct Compare {
		bool operator()(SearchState* lhs, SearchState* rhs) const {
			if (lhs->f > rhs->f) return true;
			if (lhs->f == rhs->f) return (lhs->g < rhs->g);
			return false;
		}
	};
	SearchState() {}
	SearchState(SearchState& s);
	virtual ~SearchState();
	virtual SearchState* clone() = 0;
	virtual void print() = 0;
	virtual void expand() = 0;

	virtual std::string hash() = 0;
	virtual int h() const = 0;
	virtual bool goal() const = 0;
	int f = 0;
	int g = 0;
	Solution curPath;
	PQ kids;
};

SearchState::SearchState(SearchState& s) {
	f = s.f;
	g = s.g;
	curPath = s.curPath;
}

SearchState::~SearchState() {
	while (!kids.empty())
	{
		SearchState* k = kids.top();
		kids.pop();
		delete k;
	}
}

struct SearchSpace {
public:
	~SearchSpace();
	void printInit() const;
	virtual SearchSpace* clone() = 0;
	SearchState* init = nullptr;
};

SearchSpace::~SearchSpace() {
	if (init != nullptr)
		delete init;
}

void SearchSpace::printInit() const {
	init->print();
}

class SearchAlg {
public:
	virtual ~SearchAlg();
	virtual Solution solve() = 0;
	SearchSpace* dom = nullptr;
	Solution best;
};

SearchAlg::~SearchAlg() {
	std::cout << "~SearchAlg()" << std::endl;
	if (dom != nullptr)
	{
		std::cout << " Deleting dom..." << std::endl;
		delete dom;
	}
}

#endif SEARCH_H