#ifndef SOLITAIRE_H
#define SOLITAIRE_H

#include "search.h"
#include <stack>
#include <string>

struct Solitaire : public SearchDomain {
	enum {
		Npos = 13,
		Nrank = 13,
		Nsuit = 4,
		Ncards = Nrank * Nsuit,
		Ndeck = 24,
		Nfound = 4,
		Ntab = 7,

		pDeck = 0,
		pPile = 1,
		pFound = 2,
		pTab = 6
	};

	static const int DISTANCE[Npos][Npos];
	static const char SUITS[Nsuit];
	static const char RANKS[Nrank];

	struct State : public SearchState {
		State(std::ifstream& in);
		State(const State& s);

	private:
		virtual bool goal();
		virtual int h();
		virtual void expand();
		virtual std::string hash();
		virtual void print();

		void drawCard();
		void restack();
		void pileToFoundation(int f1);
		void pileToTableau(int t1);
		void tabToFoundation(int t1, int f1);
		void tabToTab(int t1, int t2, int num);

		int moveNum(int t1, int t2);

		int pos, draws;
		std::stack<int> Deck;
		std::stack<int> Pile;
		std::stack<int> Found[Nfound];
		std::stack<int> TabV[Ntab];
		std::stack<int> TabH[Ntab];
	};

	Solitaire(std::ifstream& in);
	static std::string cardName(int c);
};

const int Solitaire::DISTANCE[][Npos] = {
	{ 0, 2, 4, 6, 8,10, 2, 2, 4, 4, 6, 8,10},
	{ 2, 0, 2, 4, 6, 8, 2, 2, 4, 2, 4, 6, 8},
	{ 4, 2, 0, 2, 4, 6, 4, 2, 4, 2, 2, 4, 6},
	{ 6, 4, 2, 0, 2, 4, 6, 4, 4, 2, 2, 2, 4},
	{ 8, 6, 4, 2, 0, 2, 8, 6, 6, 4, 2, 2, 2},
	{10, 8, 6, 4, 2, 0,10, 8, 8, 6, 4, 2, 2},
	{ 2, 2, 4, 6, 8,10, 0, 2, 4, 4, 6, 8,10},
	{ 2, 2, 4, 6, 8,10, 2, 0, 2, 4, 6, 8,10},
	{ 4, 2, 2, 4, 6, 8, 4, 2, 0, 2, 4, 6, 8},
	{ 4, 2, 2, 2, 4, 6, 4, 4, 2, 0, 2, 4, 6},
	{ 6, 4, 2, 2, 2, 4, 6, 4, 4, 2, 0, 2, 4},
	{ 8, 6, 4, 2, 2, 2, 8, 6, 6, 4, 2, 0, 2},
	{10, 8, 6, 4, 2, 2,10, 8, 8, 6, 4, 2, 0},
};
const char Solitaire::SUITS[] = { 'S','H','C','D' };
const char Solitaire::RANKS[] = { 'A','2','3','4','5','6','7','8','9','T','J','Q','K' };

Solitaire::Solitaire(std::ifstream& in) {
	init = new State(in);
}

Solitaire::State::State(std::ifstream& in) {
	int i, j;
	int c;

	for (i = 0; i < Ndeck; i++) {
		in >> c;
		Deck.push(c);
	}
	for (i = 0; i < Ntab; i++) {
		in >> c;
		TabV[i].push(c);
		for (j = i; j > 0; j--) {
			in >> c;
			TabH[i].push(c);
		}
	}

	pos = draws = 0;
	f = g + h();
}

Solitaire::State::State(const State& s) {
	pos = s.pos;
	g = s.g;
	draws = s.draws;
	curPath = s.curPath;

	Deck = s.Deck;
	Pile = s.Pile;

	for (int i = 0; i < Nfound; i++)
		Found[i] = s.Found[i];
	for (int i = 0; i < Ntab; i++)
	{
		TabV[i] = s.TabV[i];
		TabH[i] = s.TabH[i];
	}
}

bool Solitaire::State::goal() {
	if (Deck.size() + Pile.size() > 0) return false;
	/*
	for (int i = 0; i < Ntab; i++)
		if (!TabH[i].empty()) return false;
	*/
	return true;
}

int Solitaire::State::h() {
	int total = 0;
	total += 7 * ceil(Deck.size() / 3.0);
	total += 20 * (Deck.size() + Pile.size());
	int longestMove = 0;
	if (Deck.size() + Pile.size() > 0) longestMove = 20;
	/*
	for (int i = 0; i < Ntab; i++)
		if (!TabV[i].empty() && !TabH[i].empty())
		{
			int curMove = 11 * (TabV[i].size() + 1);
			if (curMove > longestMove)
				longestMove = curMove;
			total += curMove + 22 * (TabH[i].size() - 1);
		}
	*/
	total -= longestMove;
	return total;
}

void Solitaire::State::expand() {

	// Action : Draw a card from the deck
	if (Deck.size() + Pile.size() > 0) {
		if (draws < INT_MAX)
		{
			State* newState = new State(*this);
			if (newState->Deck.empty())
				newState->restack();
			else
				newState->drawCard();
			kids.push(newState);
		}
	}

	if (Pile.size() > 0) {
		int topPile = Pile.top();

		// Action : Move a card from the pile to the foundation
		for (int i = 0; i < Nfound; i++) {
			if (Found[i].empty() && topPile % 13 == 0) {
				// Moving Ace from Pile to foundation
				State* newState = new State(*this);
				newState->pileToFoundation(i);
				kids.push(newState);
			}
			else if (Found[i].size() > 0) {
				int topFnd = Found[i].top();
				if (topPile / 13 == topFnd / 13 && topPile % 13 == topFnd % 13 + 1) {
					// Moving other card from Pile to foundation
					State* newState = new State(*this);
					newState->pileToFoundation(i);
					kids.push(newState);
				}
			}
		}

		// Action : Move a card from the pile to a tableau
		for (int i = 0; i < Ntab; i++) {
			if (TabV[i].empty() && topPile % 13 == 12) {
				// Moving King from pile to empty tableau
				State* newState = new State(*this);
				newState->pileToTableau(i);
				kids.push(newState);
			}
			else if (!TabV[i].empty()) {
				int topTab = TabV[i].top();
				if ((topPile / 13) % 2 != (topTab / 13) % 2 && topPile % 13 == topTab % 13 - 1) {
					// Moving other card from Pile to tableau
					State* newState = new State(*this);
					newState->pileToTableau(i);
					kids.push(newState);
				}
			}
		}
	}

	for (int i = 0; i < Ntab; i++)
	{
		if (!TabV[i].empty())
		{
			int topTab = TabV[i].top();

			// Action: Move a card from a tableau to the foundation
			for (int j = 0; j < Nfound; j++) {
				if (Found[j].empty() && topTab % 13 == 0) {
					//Moving Ace from Tableau to Foundation
					State* newState = new State(*this);
					newState->tabToFoundation(i, j);
					kids.push(newState);
				}
				else if (!Found[j].empty()) {
					int topFnd = Found[j].top();
					if (topTab / 13 == topFnd / 13 && topTab % 13 == topFnd % 13 + 1) {
						// Moving other card from Tableau to Foundation
						State* newState = new State(*this);
						newState->tabToFoundation(i, j);
						kids.push(newState);
					}
				}
			}

			// Action: Move a card from a tableau to a tableau
			for (int j = 0; j < Ntab; j++) {
				if (i != j) {
					if (TabV[j].empty() && topTab % 13 == 12) {
						// Moving kind from Tableau to empty Tableau
						State* newState = new State(*this);
						newState->tabToTab(i, j, 1);
						kids.push(newState);
					}
					else if (!TabV[j].empty()) {
						int num = moveNum(i, j);
						if (num > 0) {
							State* newState = new State(*this);
							newState->tabToTab(i, j, num);
							kids.push(newState);
						}
					}
				}
			}
		}
	}

}

std::string Solitaire::State::hash() {
	std::string s = "d";
	std::stack<int> temp;
	temp = Deck;
	int c;
	while (!temp.empty()) {
		c = temp.top();
		temp.pop();
		s += cardName(c);
	}
	s += "p";
	temp = Pile;
	while (!temp.empty()) {
		c = temp.top();
		temp.pop();
		s += cardName(c);
	}
	for (int i = 0; i < Nfound; i++)
	{
		temp = Found[i];
		s += "f";
		while (!temp.empty()) {
			c = temp.top();
			temp.pop();
			s += cardName(c);
		}
	}
	for (int i = 0; i < Ntab; i++)
	{
		temp = TabV[i];
		s += "v";
		while (!temp.empty()) {
			c = temp.top();
			temp.pop();
			s += cardName(c);
		}
		temp = TabH[i];
		s += "h";
		while (!temp.empty()) {
			c = temp.top();
			temp.pop();
			s += cardName(c);
		}
	}

	return s;
}

void Solitaire::State::print() {
	std::cout << "Pos: " << pos << "\n";
	//system("pause");

	if (!Deck.empty())
	{
		std::cout << "Deck:\n";
		std::stack<int> temp = Deck;
		while (!temp.empty())
		{
			std::cout << cardName(temp.top()) << " ";
			temp.pop();
		}
		std::cout << "\n";
	}

	if (!Pile.empty())
	{
		std::cout << "Pile:\n";
		std::stack<int> temp = Pile;
		while (!temp.empty())
		{
			std::cout << cardName(temp.top()) << " ";
			temp.pop();
		}
		std::cout << "\n";
	}

	for (int i = 0; i < Nfound; i++)
		if (!Found[i].empty())
		{
			std::cout << "F" << i << ": ";
			std::stack<int> temp = Found[i];
			while (!temp.empty())
			{
				std::cout << cardName(temp.top()) << " ";
				temp.pop();
			}
			std::cout << "\n";
		}

	for (int i = 0; i < Ntab; i++)
	{
		if (!TabV[i].empty())
		{
			std::cout << "T" << i << ": ";
			std::stack<int> temp = TabV[i];
			while (!temp.empty())
			{
				std::cout << " " << cardName(temp.top()) << "  ";
				temp.pop();
			}
			temp = TabH[i];
			while (!temp.empty())
			{
				std::cout << "(" << cardName(temp.top()) << ") ";
				temp.pop();
			}
			std::cout << "\n";
		}
	}
}

void Solitaire::State::drawCard() {
	//std::cout << "Drawing cards..." << std::endl;

	g += DISTANCE[pos][pDeck];
	//std::cout << "Pos: " << pos << std::endl;
	//std::cout << "Added " << DISTANCE[pos][pDeck] << " to cost" << std::endl;
	int totalSize = Deck.size() + Pile.size();
	switch (totalSize) {
	case 1:
		g += 7; break;
	case 2:
		g += 8; break;
	default:
		g += 9; break;
	}
	for (int i = 0; i < 3; i++) {
		int c = Deck.top();
		Deck.pop();
		Pile.push(c);
		if (Deck.empty())
			break;
	}
	draws++;
	curPath.push_back("DRAW");
	f = g + h();
	pos = pPile;
}

void Solitaire::State::restack() {
	//std::cout << "Restacking the deck..." << std::endl;

	g += DISTANCE[pos][pDeck];
	g += 7;
	while (Pile.size() > 0) {
		int c = Pile.top();
		Pile.pop();
		Deck.push(c);
	}
	draws++;
	curPath.push_back("RESTACK");
	f = g + h();
	pos = pPile;
}

void Solitaire::State::pileToFoundation(int f1) {
	//std::cout << "Moving " << cardName(Pile.top()) << " from Pile to F" << f1 << std::endl;

	g += DISTANCE[pos][pPile] + DISTANCE[pPile][pFound + f1];

	int c = Pile.top();
	Pile.pop();
	Found[f1].push(c);

	if (!goal()) {
		g += 20;
		if (Pile.size() >= 3)
			g += 4;
	}

	draws = 0;
	std::string temp = "P->F";
	temp += std::to_string(f1);
	curPath.push_back(temp);
	f = g + h();
	pos = pFound + f1;
}

void Solitaire::State::pileToTableau(int t1) {
	//std::cout << "Moving " << cardName(Pile.top()) << " from Pile to T" << t1 << std::endl;

	g += DISTANCE[pos][pPile] + DISTANCE[pPile][pTab + t1];

	int c = Pile.top();
	Pile.pop();
	TabV[t1].push(c);

	if (!goal()) {
		g += 20;
		if (Pile.size() >= 3)
			g += 4;
	}

	draws = 0;
	std::string temp = "P->T";
	temp += std::to_string(t1);
	curPath.push_back(temp);
	f = g + h();
	pos = pTab + t1;
}

void Solitaire::State::tabToFoundation(int t1, int f1) {
	//std::cout << "Moving " << cardName(TabV[t1].top()) << " from T" << t1 << " to F" << f1 << std::endl;

	g += DISTANCE[pos][pTab + t1] + DISTANCE[pTab + t1][pFound + f1];

	int c = TabV[t1].top();
	TabV[t1].pop();
	Found[f1].push(c);
	// Reveal hidden card
	if (!TabH[t1].empty()) {
		int next = TabH[t1].top();
		TabH[t1].pop();
		TabV[t1].push(next);
	}

	if (!goal()) {
		g += 20;
		if (Found[f1].size() > 1)
			g += 4;
	}

	draws = 0;
	std::string temp = "T";
	temp += std::to_string(t1);
	temp += "->F";
	temp += std::to_string(f1);
	curPath.push_back(temp);
	f = g + h();
	pos = pFound + f1;
}

void Solitaire::State::tabToTab(int t1, int t2, int num) {
	//std::cout << "Moving " << num << " cards from T" << t1 << " to T" << t2 << std::endl;

	g += DISTANCE[pos][pTab + t1] + DISTANCE[pTab + t1][pTab + t2];

	std::stack<int> temp;
	for (int i = 0; i < num; i++)
	{
		int c = TabV[t1].top();
		TabV[t1].pop();
		temp.push(c);
	}

	for (int i = 0; i < num; i++)
	{
		int c = temp.top();
		temp.pop();
		TabV[t2].push(c);
	}
	//Reveal hidden card
	if (TabV[t1].size() == 0 && TabH[t1].size() > 0)
	{
		int next = TabH[t1].top();
		TabH[t1].pop();
		TabV[t1].push(next);
	}

	if (!goal()) {
		g += 11 * (num + 1);
	}

	draws = 0;
	std::string temps = "T";
	temps += std::to_string(t1);
	temps += "->T";
	temps += std::to_string(t2);
	curPath.push_back(temps);
	f = g + h();
	pos = pTab + t2;
}

int Solitaire::State::moveNum(int t1, int t2) {
	int targetCard = TabV[t2].top();
	int numTotal = TabV[t1].size();
	int counter = 0;
	std::stack<int> temp = TabV[t1];
	while (!temp.empty()) {
		int curCard = temp.top();
		temp.pop();
		counter++;
		if ((curCard / 13) % 2 != (targetCard / 13) % 2 && curCard % 13 == targetCard % 13 - 1) {
			if (counter == numTotal) return numTotal;
			return 0;
		}
	}
	return 0;
}

std::string Solitaire::cardName(int c) {
	std::string s = "";
	int suit = c / Nrank;
	int rank = c % Nrank;
	s += RANKS[rank];
	s += SUITS[suit];
	return s;
}

#endif SOLITAIRE_H