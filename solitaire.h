#ifndef SOLITAIRE_H
#define SOLITAIRE_H

#include <stack>
#include <string>

class Solitaire : public SearchSpace {

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

	class State : public SearchState {
	public:
		State(std::ifstream& in);
		State(State& s);
		SearchState* clone();
		virtual void print();
		virtual void expand();

		bool goal() const;
		int h() const;
		virtual std::string hash();

	private:
		void restack();
		void drawCard();
		void pileToFoundation(int f1);
		void pileToTableau(int t1);
		void tabToFoundation(int t1, int f1);
		void tabToTab(int t1, int t2, int num);
		int moveNum(int t1, int t2);

		int pos = 0;
		int draws = 0;
		std::stack<int> Deck;
		std::stack<int> Pile;
		std::vector<std::stack<int>> Found;
		std::vector<std::stack<int>> TabV;
		std::vector<std::stack<int>> TabH;
	};

public:
	Solitaire(std::ifstream& in);
	Solitaire(Solitaire& s);
	SearchSpace* clone();
	static std::string cardName(int c);
private:
};

const int Solitaire::DISTANCE[][Npos] = {
	{ 0, 1, 3, 5, 7, 9, 1, 1, 3, 3, 5, 7, 9},
	{ 1, 0, 1, 3, 5, 7, 1, 1, 3, 1, 3, 5, 7},
	{ 3, 1, 0, 1, 3, 5, 3, 1, 3, 1, 1, 3, 5},
	{ 5, 3, 1, 0, 1, 3, 5, 3, 3, 1, 1, 1, 3},
	{ 7, 5, 3, 1, 0, 1, 7, 5, 5, 3, 1, 1, 1},
	{ 9, 7, 5, 3, 1, 0, 9, 7, 7, 5, 3, 1, 1},
	{ 1, 1, 3, 5, 7, 9, 0, 1, 3, 3, 5, 7, 9},
	{ 1, 1, 3, 5, 7, 9, 1, 0, 1, 3, 5, 7, 9},
	{ 3, 1, 1, 3, 5, 7, 3, 1, 0, 1, 3, 5, 7},
	{ 3, 1, 1, 1, 3, 5, 3, 3, 1, 0, 1, 3, 5},
	{ 5, 3, 1, 1, 1, 3, 5, 3, 3, 1, 0, 1, 3},
	{ 7, 5, 3, 1, 1, 1, 7, 5, 5, 3, 1, 0, 1},
	{ 9, 7, 5, 3, 1, 1, 9, 7, 7, 5, 3, 1, 0},
};
const char Solitaire::SUITS[] = { 'S','H','C','D' };
const char Solitaire::RANKS[] = { 'A','2','3','4','5','6','7','8','9','T','J','Q','K' };

std::string Solitaire::State::hash() {
	std::string s = "p";
	s += std::to_string(pos);
//	s += "d";
//	s += std::to_string(draws);
	s += "d";
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

Solitaire::Solitaire(std::ifstream& in) {
	init = new State(in);
}

Solitaire::Solitaire(Solitaire& s) {
	State* st = (State*)s.init;
	init = new State(*st);
}

SearchState* Solitaire::State::clone() {
	return new State(*this);
}

SearchSpace* Solitaire::clone() {
	return new Solitaire(*this);
}

std::string Solitaire::cardName(int c) {
	std::string s = "";
	int suit = c / Nrank;
	int rank = c % Nrank;
	s += RANKS[rank];
	s += SUITS[suit];
	return s;
}

void Solitaire::State::expand() {

	// Action : Draw a card from the deck
	if (Deck.size() + Pile.size() > 0) {
		if (draws < 2)
		{
			State* newState = new State(*this);
			if (newState->Deck.empty())
				newState->restack();
			else
				newState->drawCard();
			kids.push(newState);
		}
	}

	if (Pile.size() > 0 && draws < 2) {
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

Solitaire::State::State(std::ifstream& in) {
	int i, j;
	int c;

	Found.resize(Nfound);
	TabV.resize(Ntab);
	TabH.resize(Ntab);

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
	pos = 0;
	f = g + h();
}

Solitaire::State::State(State& s) : SearchState(s) {
	pos = s.pos;
	draws = s.draws;
	g = s.g;
	curPath = s.curPath;

	Deck = s.Deck;
	Pile = s.Pile;
	Found = s.Found;
	TabV = s.TabV;
	TabH = s.TabH;
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

int Solitaire::State::h() const {
	int total = 0;
	total += 7 * ceil(Deck.size() / 3.0);
	total += 20 * (Deck.size() + Pile.size());
	int longestMove = 0;
	if (Deck.size() + Pile.size() > 0) longestMove = 20;
	for (int i = 0; i < Ntab; i++)
		if (!TabV[i].empty() && !TabH[i].empty())
		{
			int curMove = 11 * (TabV[i].size() + 1);
			if (curMove > longestMove)
				longestMove = curMove;
			total += curMove + 22 * (TabH[i].size() - 1);
		}
	total -= longestMove;
	return total;
}

bool Solitaire::State::goal() const {
	if (Deck.size() + Pile.size() > 0) return false;
	for (int i = 0; i < Ntab; i++)
		if (!TabH[i].empty()) return false;
	return true;
}


#endif SOLITAIRE_H