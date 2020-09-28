#ifndef SEARCH_H
#define SEARCH_H

#include <iostream>
#include <queue>
#include <string>
#include <vector>

struct Solution : public std::vector<std::string> {
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

	virtual bool goal() = 0;
	virtual int h() = 0;
	virtual void expand() = 0;
	virtual std::string hash() = 0;
	virtual void print() = 0;

	int f = 0;
	int g = 0;
	Solution curPath;
	std::priority_queue<SearchState*, std::vector<SearchState*>, Compare> kids;
};

struct SearchDomain {
	~SearchDomain();
	void printInit() const;
	SearchState* init;
};

SearchDomain::~SearchDomain() {
	delete init;
}

void SearchDomain::printInit() const {
	init->print();
}

struct SearchAlg {
	~SearchAlg();
	virtual Solution solve() = 0;
	virtual void solve(SearchState* s) = 0;
	SearchDomain* dom;
	Solution best;
};

SearchAlg::~SearchAlg() {
	delete dom;
}

#endif SEARCH_H