#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <queue>
#include <map>

const int MAX_FRAMES = 9999;

const int DISTANCE[13][13] = {
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

const char SUITS[4] = {
	'S','H','C','D'
};

const char RANKS[13] = {
	'A','2','3','4','5','6','7','8','9','T','J','Q','K'
};

struct Solitaire {

	enum {
		Npos = 13,
		Nranks = 13,
		Nsuits = 4,
		Ncards = Nranks * Nsuits,
		Ndeck = 24,
		Nfound = 4,
		Ntab = 7
	};

	/*
	const char SUITS[Nsuits] = {
		'S','H','C','D'
	};

	const char RANKS[Nranks] = {
		'A','2','3','4','5','6','7','8','9','T','J','Q','K'
	};
	*/

	struct State {
		enum {
			pDeck = 0,
			pPile = 1,
			pFound = 2,
			pTabs = 6
		};
		int Pos;
		int g;
		int h;
		int draws;

		std::vector<std::string> Moves;

		std::stack<int> Deck;
		std::stack<int> Pile;
		std::stack<int> Foundations[Nfound];
		std::stack<int> Tableaux_Visible[Ntab];
		std::stack<int> Tableaux_Hidden[Ntab];

		State() {}

		State(const State& s) {
			Pos = s.Pos;
			g = s.g;
			h = s.h;
			draws = s.draws;
			Moves = s.Moves;
			Deck = s.Deck;
			Pile = s.Pile;
			for (int i = 0; i < Nfound; i++)
				Foundations[i] = s.Foundations[i];
			for (int i = 0; i < Ntab; i++)
			{
				Tableaux_Visible[i] = s.Tableaux_Visible[i];
				Tableaux_Hidden[i] = s.Tableaux_Hidden[i];
			}
		}

		bool Goal() {
			return (h == 0);
		}

		int hVal() {
			//return 7 * ceil(deckSize() / 3.0) + 20 * (deckSize() + pileSize() - 12);

			int total = 0;
			total += 7 * ceil(deckSize() / 3.0);
			total += 20 * (deckSize() + pileSize());
			int longestMove = 0;
			if (deckSize() - pileSize() > 0) longestMove = 20;
			for (int i = 0; i < 7; i++)
				if (tV_Size(i) > 0 && tH_Size(i) > 0)
				{
					int curMove = 11 * (tV_Size(i) + 1);
					if (curMove > longestMove)
						longestMove = curMove;
					total += curMove + 22 * ((tH_Size(i) - 1));
				}

			// Subtract the time it takes for the final action since it happens after last input
			total -= longestMove;

			return total;
		}

		std::priority_queue<Solitaire::State> getKids() {
			std::priority_queue<Solitaire::State> kids;

			// Test all possible moves


			// Action: Draw a card from the deck
			if (deckSize() + pileSize() > 0) {
				// Don't draw more than 8 times between pile moves
				if (draws < 16)
				{
					//std::cout << draws << std::endl;
					State newState(*this);
						newState.g += DISTANCE[Pos][pDeck];
						// Deck is empty
						if (newState.deckSize() == 0) {
							newState.g += 7;
								newState.restack();
						}
					// Deck is not empty
						else
						{
							if (newState.deckSize() + newState.pileSize() == 1)
								newState.g += 7;
							else if (newState.deckSize() + newState.pileSize() == 2)
								newState.g += 8;
							else
								newState.g += 9;

							newState.drawCard();
						}
					newState.Pos = pPile;
					newState.Moves.push_back("DRAW");
					newState.h = hVal();
					//std::cout << "DRAW" << std::endl;
					//std::cout << "Creating new DRAW move w/ cost " << newState.g - g << "\n";
					newState.draws = draws + 1;
					kids.push(newState);
				}
			}

			// Action: Move a card from the pile to the foundation
			if (pileSize() > 0)
			{
				int topCard = Pile.top();
				for (int i = 0; i < Nfound; i++)
				{
					if (Foundations[i].size() == 0 && topCard % 13 == 0)
					{
						// Moving Ace from Pile to foundation
						State newState(*this);
						newState.pileToFoundation(i);

						// Pick target/destination in either order
						newState.g += std::min(DISTANCE[Pos][pPile] + DISTANCE[pPile][pFound + i],
							                   DISTANCE[Pos][pFound + i] + DISTANCE[pFound + i][pPile]);

						newState.h = hVal();
						if (!newState.Goal())
						{
							newState.g += 20;
							if (newState.pileSize() >= 3)
								newState.g += 4;
						}

						newState.Pos = pFound + i;
						std::string temp = "P";
						//temp += "(";
						//temp += RANKS[topCard % 13];
						//temp += SUITS[topCard / 13];
						//temp += ")"; 
						temp += "->F";
						temp += std::to_string(i);
						newState.Moves.push_back(temp);
						//std::cout << temp << std::endl;
						//std::cout << "Creating new P->F move w/ cost " << newState.g - g << "\n";
						newState.draws = 0;
						kids.push(newState);

					}
					else if (Foundations[i].size() > 0) {
						// Moving card from Pile to foundation
						int topFnd = Foundations[i].top();
						if (topCard / 13 == topFnd / 13 && topCard % 13 == topFnd % 13 + 1) {
							State newState(*this);
							newState.pileToFoundation(i);

							// Pick target/destination in either order
							newState.g += std::min(DISTANCE[Pos][pPile] + DISTANCE[pPile][pFound + i],
								DISTANCE[Pos][pFound + i] + DISTANCE[pFound + i][pPile]);

							newState.h = hVal();
							if (!newState.Goal())
							{
								newState.g += 20;
								if (newState.pileSize() >= 3)
									newState.g += 4;
							}

							newState.Pos = pFound + i;
							std::string temp = "P";
							//temp += "(";
							//temp += RANKS[topCard % 13];
							//temp += SUITS[topCard / 13];
							//temp += ")"; 
							temp += "->F";
							temp += std::to_string(i);
							newState.Moves.push_back(temp);
							//std::cout << temp << std::endl;
							//std::cout << "Creating new P->F move w/ cost " << newState.g - g << "\n";
							newState.draws = 0;
							kids.push(newState);
						}
					}
				}
			}

			// Action: Move a card from the pile to a tableau
			if (pileSize() > 0)
			{
				int topCard = Pile.top();
				for (int i = 0; i < Ntab; i++)
				{
					if (Tableaux_Visible[i].size() == 0 && topCard % 13 == 12)
					{
						// Moving King from pile to empty tableau
						State newState(*this);
						newState.pileToTableau(i);

						// Pick target/destination in either order
						newState.g += std::min(DISTANCE[Pos][pPile] + DISTANCE[pPile][pTabs + i],
							DISTANCE[Pos][pTabs + i] + DISTANCE[pTabs + i][pPile]);

						newState.h = hVal();
						if (!newState.Goal())
						{
							newState.g += 20;
							if (newState.pileSize() >= 3)
								newState.g += 4;
						}

						newState.Pos = pTabs + i;
						std::string temp = "P";
					//	temp += "(";
					//	temp += RANKS[topCard % 13];
					//	temp += SUITS[topCard / 13];
					//	temp += ")";
						temp += "->T";
						temp += std::to_string(i);

						newState.Moves.push_back(temp);
						//std::cout << temp << std::endl;
						//std::cout << "Creating new P->T move w/ cost " << newState.g - g << "\n";
						newState.draws = 0;
						kids.push(newState);
					}
					else if (Tableaux_Visible[i].size() > 0)
					{
						int tabCard = Tableaux_Visible[i].top();
						if ((topCard/13)%2 != (tabCard/13)%2 && topCard % 13 == tabCard % 13 - 1) {
							// Moving card from pile to tableau
							State newState(*this);
							newState.pileToTableau(i);

							// Pick target/destination in either order
							newState.g += std::min(DISTANCE[Pos][pPile] + DISTANCE[pPile][pTabs + i],
								DISTANCE[Pos][pTabs + i] + DISTANCE[pTabs + i][pPile]);

							newState.h = hVal();
							if (!newState.Goal())
							{
								newState.g += 20;
								if (newState.pileSize() >= 3)
									newState.g += 4;
							}

							newState.Pos = pTabs + i;
							std::string temp = "P";
					//		temp += "(";
					//		temp += RANKS[topCard % 13];
					//		temp += SUITS[topCard / 13];
					//		temp += ")"; 
							temp += "->T";
							temp += std::to_string(i);
							newState.Moves.push_back(temp);
							//std::cout << temp << std::endl;
							//std::cout << "Creating new P->T move w/ cost " << newState.g - g << "\n";
							newState.draws = 0;
							kids.push(newState);
						}
					}
				}
			}

			// Action: Move a card from a tableau to the foundation
			for (int i = 0; i < Ntab; i++)
			{
				if (Tableaux_Visible[i].size() > 0) {
					int topCard = Tableaux_Visible[i].top();
					for (int j = 0; j < Nfound; j++)
					{
						if (Foundations[j].size() == 0 && topCard % 13 == 0)
						{
							// Moving Ace from Tableau to Foundation
							State newState(*this);
							newState.tabToFoundation(i,j);

							// Pick target/destination in either order
							newState.g += std::min(DISTANCE[Pos][pTabs+i] + DISTANCE[pTabs+i][pFound+j],
								DISTANCE[Pos][pFound+j] + DISTANCE[pFound+j][pTabs+i]);

							newState.h = hVal();
							if (!newState.Goal())
							{
								newState.g += 20;
							}

							newState.Pos = pFound + j;

							std::string temp = "T";
							temp += std::to_string(i);
					//		temp += "(";
					//		temp += RANKS[topCard % 13];
					//		temp += SUITS[topCard / 13];
					//		temp += ")";
							temp += "->F";
							temp += std::to_string(j);
							newState.Moves.push_back(temp);
							//std::cout << temp << std::endl;
							//std::cout << "Creating new T->F move w/ cost " << newState.g - g << "\n";
							kids.push(newState);
						}
						else if (Foundations[j].size() > 0)
						{
							int topFnd = Foundations[j].top();
							if (topCard / 13 == topFnd / 13 && topCard % 13 == topFnd % 13 + 1) {
								// Moving card from Tableau to Foundation
								State newState(*this);
								newState.tabToFoundation(i, j);

								// Pick target/destination in either order
								newState.g += std::min(DISTANCE[Pos][pTabs + i] + DISTANCE[pTabs + i][pFound + j],
									DISTANCE[Pos][pFound + j] + DISTANCE[pFound + j][pTabs + i]);

								newState.h = hVal();
								if (!newState.Goal())
								{
									newState.g += 24;
								}

								newState.Pos = pFound + j;

								std::string temp = "T";
								temp += std::to_string(i);
						//		temp += "(";
						//		temp += RANKS[topCard % 13];
						//		temp += SUITS[topCard / 13];
						//		temp += ")";
								temp += "->F";
								temp += std::to_string(j);
						//		temp += "(";
						//		temp += RANKS[topFnd % 13];
						//		temp += SUITS[topFnd / 13];
						//		temp += ")";
								newState.Moves.push_back(temp);
								//std::cout << temp << std::endl;
								//std::cout << "Creating new T->F move w/ cost " << newState.g - g << "\n";
								kids.push(newState);
							}

						}
					}
				}
			}

			// Action: Move a card from a tableau to a tableau
			for (int i = 0; i < Ntab; i++)
			{
				if (Tableaux_Visible[i].size() > 0)
				{
					int topCard = Tableaux_Visible[i].top();
					for (int j = 0; j < Ntab; j++)
					{
						if (i != j)
						{
							if (Tableaux_Visible[j].size() == 0 && topCard % 13 == 12)
							{
								// Moving King from Tableau to Tableau
								State newState(*this);
								newState.tabToTab(i, j, 1);

								// Pick target/destination in either order
								newState.g += std::min(DISTANCE[Pos][pTabs + i] + DISTANCE[pTabs + i][pTabs + j],
									DISTANCE[Pos][pTabs + j] + DISTANCE[pTabs + j][pTabs + i]);

								newState.h = hVal();
								if (!newState.Goal())
								{
									newState.g += 22;
								}

								newState.Pos = pTabs + j;

								std::string temp = "T";
								temp += std::to_string(i);
						//		temp += "(";
						//		temp += RANKS[topCard % 13];
						//		temp += SUITS[topCard / 13];
						//		temp += ")";
								temp += "->T";
								temp += std::to_string(j);
								newState.Moves.push_back(temp);
								//std::cout << temp << std::endl;
								//std::cout << "Creating new T->T move w/ cost " << newState.g - g << "\n";
								kids.push(newState);
							}
							else if (Tableaux_Visible[j].size() > 0)
							{
								int num = moveNum(i, j);
								if (num > 0)
								{
									State newState(*this);
									newState.tabToTab(i, j, num);

									// Pick target/destination in either order
									newState.g += std::min(DISTANCE[Pos][pTabs + i] + DISTANCE[pTabs + i][pTabs + j],
										DISTANCE[Pos][pTabs + j] + DISTANCE[pTabs + j][pTabs + i]);

									newState.h = hVal();
									if (!newState.Goal())
									{
										newState.g += 11 * (num + 1);
									}

									newState.Pos = pTabs + j;

									std::string temp = "T";
									temp += std::to_string(i);
									temp += "->T";
									temp += std::to_string(j);
							//		temp += " (";
						//			temp += std::to_string(num);
						//			temp += " cards)";
									//temp += "[tV is now ";
									//temp += std::to_string(newState.Tableaux_Visible[i].size());
									//temp += ", tH is now ";
									//temp += std::to_string(newState.Tableaux_Hidden[i].size());
									//temp += "]";
									newState.Moves.push_back(temp);
									//std::cout << temp << std::endl;
									//std::cout << "Creating new T->T move w/ cost " << newState.g - g << "\n";
									kids.push(newState);
								}
							}
						}
					}
				}
			}


			return kids;
		}

				std::string hash() {
			std::string s = "d ";
			std::stack<int> temp = Deck;
			for (int i = 0; i < deckSize(); i++)
			{
				int card = temp.top();
				temp.pop();
				s += std::to_string(card);
				s += " ";
			}

			s += "p ";
			temp = Pile;
			for (int i = 0; i < pileSize(); i++)
			{
				int card = temp.top();
				temp.pop();
				s += std::to_string(card);
				s += " ";
			}

			s += "f ";
			for (int i = 0; i < 4; i++)
			{
				std::stack<int> temp = Foundations[i];
				for (int j = 0; j < temp.size(); j++)
				{
					int card = temp.top();
					temp.pop();
					s += std::to_string(card);
					s += " ";
				}
				s += ",";
			}

			s += "tv ";
			for (int i = 0; i < 7; i++)
			{
				std::stack<int> temp = Tableaux_Visible[i];
				for (int j = 0; j < temp.size(); j++)
				{
					int card = temp.top();
					temp.pop();
					s += std::to_string(card);
					s += " ";
				}
				s += ",";
			}

			s += "th ";
			for (int i = 0; i < 7; i++)
			{
				std::stack<int> temp = Tableaux_Hidden[i];
				for (int j = 0; j < temp.size(); j++)
				{
					int card = temp.top();
					temp.pop();
					s += std::to_string(card);
					s += " ";
				}
				s += ",";
			}

			return s;

		}

	private:



		int moveNum(int t1, int t2) {
			int targetCard = Tableaux_Visible[t2].top();
			int numTotal = Tableaux_Visible[t1].size();
			int counter = 0;
			std::stack<int> temp = Tableaux_Visible[t1];
			while (!temp.empty())
			{
				int curCard = temp.top();
				temp.pop();
				counter++;
				if ((curCard / 13) % 2 != (targetCard / 13) % 2 && curCard % 13 == targetCard % 13 - 1)
				{
					if (counter == numTotal) return numTotal;
					return 0;
				}
			}

			return 0;
		}

		void tabToTab(int t1, int t2, int num) {

			//std::cout << "Now performing tabToTab" << std::endl;

			std::stack<int> temp;
			
			for (int i = 0; i < num; i++)
			{
				int c = Tableaux_Visible[t1].top();
				Tableaux_Visible[t1].pop();
				temp.push(c);
			}
			
			for (int i = 0; i < num; i++)
			{
				int c = temp.top();
				temp.pop();
				Tableaux_Visible[t2].push(c);
			}
			//Reveal hidden card
			if (Tableaux_Visible[t1].size() == 0 && Tableaux_Hidden[t1].size() > 0)
			{
				int next = Tableaux_Hidden[t1].top();
				Tableaux_Hidden[t1].pop();
				Tableaux_Visible[t1].push(next);
			}
		}

		void tabToFoundation(int t, int f) {
			int c = Tableaux_Visible[t].top();
			Tableaux_Visible[t].pop();
			Foundations[f].push(c);

			//Reveal hidden card
			if (Tableaux_Visible[t].size() == 0 && Tableaux_Hidden[t].size() > 0)
			{
				int next = Tableaux_Hidden[t].top();
				Tableaux_Hidden[t].pop();
				Tableaux_Visible[t].push(next);
			}
		}

		void pileToTableau(int f) {
			int c = Pile.top();
			Pile.pop();
			Tableaux_Visible[f].push(c);
		}

		void pileToFoundation(int f) {
			int c = Pile.top();
			Pile.pop();
			Foundations[f].push(c);
		}

		void restack() {
			while (pileSize() > 0) {
				int nextCard = Pile.top();
				Pile.pop();
				Deck.push(nextCard);
			}
		}

		void drawCard() {
			for (int i = 0; i < 3; i++) {
				int nextCard = Deck.top();
				Deck.pop();
				Pile.push(nextCard);
				if (deckSize() == 0) break;
			}
		}

		int deckSize() {
			return Deck.size();
		}

		int pileSize() {
			return Pile.size();
		}

		int tV_Size(int t) {
			return Tableaux_Visible[t].size();
		}

		int tH_Size(int t) {
			return Tableaux_Hidden[t].size();
		}

	};

	Solitaire(std::ifstream& in);
	State Init();
	void Solve(State s);

	//Debug
	void print();

	int BestFrames;
	std::vector<std::string> BestMoveList;
	std::map<std::string, int> StateList;

private:
	int init[Ncards];
	std::string cardName(int c);
};

bool operator< (const Solitaire::State& state1, const Solitaire::State& state2) {
	if (state1.g+state1.h > state2.g+state2.h) return true;
	if (state1.g+state1.h == state2.g+state2.h) return (state1.g < state2.g);
	return false;
}

Solitaire::Solitaire(std::ifstream& in) {
	for (int i = 0; i < Ncards; i++)
		in >> init[i];
	BestFrames = MAX_FRAMES;
}

Solitaire::State Solitaire::Init() {
	State s;
	s.Pos = 0;
	s.draws = 0;

	for (int i = Ndeck - 1; i >= 0; i--)
		s.Deck.push(init[i]);

	for (int i = 0; i < Ntab; i++)
	{
		s.Tableaux_Visible[i].push(init[Ndeck + (i) * (i + 1) / 2]);
		for (int j = i; j > 0; j--)
			s.Tableaux_Hidden[i].push(init[Ndeck + (i) * (i + 1) / 2 + j]);
	}

	s.g = 0;
	s.h = s.hVal();

	return s;
}

void Solitaire::Solve(Solitaire::State s) {
	if (s.Goal()) {
		std::cout << "New best path found @ " << s.g << " frames\n";
		BestFrames = s.g;
		BestMoveList = s.Moves;
		for (int i = 0; i < BestMoveList.size(); i++)
			std::cout << BestMoveList[i] << " ";
		std::cout << "\n";

		//system("pause");
		return;
	}

	std::string curHash = s.hash();
	int count = StateList.count(curHash);
	if (count > 0)
	{
		int prevF = StateList[curHash];
		if (s.g + s.h >= prevF)
		{
			//std::cout << "Skipping repeat state:" << std::endl;
			return;
		}
	}

	StateList[curHash] = s.g + s.h;
	//std::cout << "Cur fVal:" << s.g + s.h << "\n";
	std::priority_queue<Solitaire::State> kids = s.getKids();
	while (!kids.empty())
	{
		Solitaire::State curKid = kids.top();
		kids.pop();
		if (curKid.g + curKid.h >= BestFrames) break;
		Solve(curKid);
	}
}

void Solitaire::print() {
	std::cout << "Deck:\n";
	for (int i = 0; i < Ndeck; i++) {
		std::cout << cardName(init[i]) << " ";
	}
	std::cout << "\n\n";
	std::cout << "Tableaux:\n";
	for (int i = 0; i < Ntab; i++) {
		for (int j = 0; j < i + 1; j++)
			std::cout << cardName(init[Ndeck + (i)*(i+1)/2 + j]) << " ";
		std::cout << "\n";
	}
}

std::string Solitaire::cardName(int c) {
	std::string s = "";
	int suit = c / Nranks;
	int rank = c % Nranks;
	s += RANKS[rank];
	s += SUITS[suit];
	return s;
}

int main(int argc, const char *argv[]) {

	std::ifstream in;
	in.open("instance.txt");
	Solitaire solitaire(in);
	in.close();
	solitaire.print();
	system("pause");

	Solitaire::State init = solitaire.Init();
	solitaire.Solve(init);
	std::cout << std::endl;
	std::cout << solitaire.BestFrames << std::endl;
	for (int i = 0; i < solitaire.BestMoveList.size(); i++)
		std::cout << solitaire.BestMoveList[i] << " ";
	std::cout << std::endl;

	return 0;
}