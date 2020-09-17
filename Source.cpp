#include <stack>
#include <string>
#include <queue>
#include <vector>
#include <iostream>
using namespace std;

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

class Card {
public:
	Card(const string r, const string s);
	string getRank() const;
	string getSuit() const;
private:
	string rank;
	string suit;
};

Card::Card(const string r, const string s) {
	rank = r;
	suit = s;
}

string Card::getRank() const {
	return rank;
}

string Card::getSuit() const {
	return suit;
}

class State {
public:
	State(const stack<Card> d, const vector<stack<Card>> tV, const vector<stack<Card>> tH);
	State(const State &prev);

	vector<string> moveList;
	
	int position;
	int cardsRemaining;

	int fVal;
	int gVal;
	int hVal();

	int deckSize();
	int pileSize();

	Card topPile();
	bool fndEmpty(int f);
	bool tabEmpty(int f);
	Card topFnd(int f);
	Card topTab(int f);

	void drawCard();
	void restack();
	void pileToFoundation(int f);
	void pileToTableau(int f);
	void tabToFoundation(int t, int f);
	void tabToTab(int t1, int t2, int num);

	int moveNum(int t1, int t2);

	bool win();

private:
	stack<Card> deck;
	stack<Card> pile;
	vector<stack<Card>> foundation;
	vector<stack<Card>> tableaux_visible;
	vector<stack<Card>> tableaux_hidden;
};

int State::hVal() {
	// Heuristic
	// Find the # of frames it takes to:
	// Deposit all of the remaining deck into the pile
	// Move all of the pile into the foundation
	// Uncover all of the hidden cards
	int total = 0;
	total += 7 * deckSize();
	total += 20 * (deckSize() + pileSize());
	for (int i = 0; i < 7; i++)
		if (tableaux_visible[i].size() > 0 && tableaux_hidden[i].size() > 0)
			total += 20 * tableaux_hidden[i].size();
	return total;
}

void State::tabToTab(int t1, int t2, int num) {
	stack<Card> temp;
	for (int i = 0; i < num; i++)
	{
		Card c = tableaux_visible[t1].top();
		tableaux_visible[t1].pop();
		temp.push(c);
	}
	for (int i = 0; i < num; i++)
	{
		Card c = temp.top();
		temp.pop();
		tableaux_visible[t2].push(c);
	}
	//Reveal hidden card
	if (tableaux_visible[t1].size() == 0 && tableaux_hidden[t1].size() > 0)
	{
		Card next = tableaux_hidden[t1].top();
		tableaux_hidden[t1].pop();
		tableaux_visible[t1].push(next);
	}
}

bool goesOnTop(Card top, Card bottom) {
	string rankTop = top.getRank();
	string rankBottom = bottom.getRank();
	string suitTop = top.getSuit();
	string suitBottom = bottom.getSuit();

	if (rankTop == "Spades" || rankTop == "Clubs")
		if (rankBottom == "Spades" || rankBottom == "Clubs")
			return false;
	if (rankTop == "Hearts" || rankTop == "Diamonds")
		if (rankBottom == "Hearts" || rankBottom == "Diamonds")
			return false;

	if (rankTop == "A") return (rankBottom == "2");
	if (rankTop == "2") return (rankBottom == "3");
	if (rankTop == "3") return (rankBottom == "4");
	if (rankTop == "4") return (rankBottom == "5");
	if (rankTop == "5") return (rankBottom == "6");
	if (rankTop == "6") return (rankBottom == "7");
	if (rankTop == "7") return (rankBottom == "8");
	if (rankTop == "8") return (rankBottom == "9");
	if (rankTop == "9") return (rankBottom == "10");
	if (rankTop == "10") return (rankBottom == "J");
	if (rankTop == "J") return (rankBottom == "Q");
	if (rankTop == "Q") return (rankBottom == "K");
	return false;
}

// Find the # of cards that will be moved from Tableau 1 to Tableau 2
int State::moveNum(int t1, int t2) {

	Card targetCard = topTab(t2);

	int counter = 0;
	stack<Card> temp = tableaux_visible[t1];
	while (!temp.empty())
	{
		Card curCard = temp.top();
		temp.pop();
		counter++;
		if (goesOnTop(curCard, targetCard)) return counter;
	}

	return 0;
}

void State::tabToFoundation(int t, int f) {
	Card c = tableaux_visible[t].top();
	tableaux_visible[t].pop();
	foundation[f].push(c);

	//Reveal hidden card
	if (tableaux_visible[t].size() == 0 && tableaux_hidden[t].size() > 0)
	{
		Card next = tableaux_hidden[t].top();
		tableaux_hidden[t].pop();
		tableaux_visible[t].push(next);
	}
		
}

void State::pileToTableau(int f) {
	Card c = pile.top();
	pile.pop();
	tableaux_visible[f].push(c);
}

Card State::topTab(int f) {
	return tableaux_visible[f].top();
}

bool State::fndEmpty(int f) {
	return foundation[f].empty();
}

bool State::tabEmpty(int f) {
	return tableaux_visible[f].empty();
}

void State::pileToFoundation(int f) {
	Card c = pile.top();
	pile.pop();
	foundation[f].push(c);
}

Card State::topFnd(int f) {
	return foundation[f].top();
}

Card State::topPile() {
	return pile.top();
}

void State::restack() {
	while (!pile.empty())
	{
		Card nextCard = pile.top();
		pile.pop();
		deck.push(nextCard);
	}
}

void State::drawCard() {
	Card nextCard = deck.top();
	deck.pop();
	pile.push(nextCard);
}

bool State::win() {
	//Input ends once autoplay takes over (all remaining cards revealed & in tableaux)
	if (cardsRemaining == 0) return true;
	if (deckSize() + pileSize() > 0) return false;
	for (int i = 0; i < 7; i++)
		if (tableaux_hidden[i].size() > 0)
			return false;
	return true;
}

bool operator< (const State& state1, const State& state2) {
	return state1.fVal > state2.fVal;
}

State::State(const stack<Card> d, const vector<stack<Card>> tV, const vector<stack<Card>> tH) {
	
	position = 0;
	cardsRemaining = 52;
	fVal = 0;
	gVal = 0;

	moveList = vector<string>();

	deck = d;

	stack<Card> newPile;
	pile = newPile;

	vector<stack<Card>> newFoundation;
	for (int i = 0; i < 4; i++)
	{
		stack<Card> newStack;
		newFoundation.push_back(newStack);
	}
	foundation = newFoundation;

	for (int i = 0; i < 7; i++)
	{
		tableaux_visible.push_back(tV[i]);
		tableaux_hidden.push_back(tH[i]);
	}
}

State::State(const State& prev) {
	position = prev.position;
	cardsRemaining = prev.cardsRemaining;

	moveList = prev.moveList;

	fVal = prev.fVal;
	gVal = prev.gVal;

	deck = prev.deck;
	pile = prev.pile;

	vector<stack<Card>> newFoundation;

	for (int i = 0; i < 4; i++) {
		newFoundation.push_back(prev.foundation[i]);
	}
	foundation = newFoundation;

	vector<stack<Card>> newTV;
	vector<stack<Card>> newTH;
	for (int i = 0; i < 7; i++) {
		newTV.push_back(prev.tableaux_visible[i]);
		newTH.push_back(prev.tableaux_hidden[i]);
	}
	tableaux_visible = newTV;
	tableaux_hidden = newTH;
}

int State::deckSize() {
	return deck.size();
}

int State::pileSize() {
	return pile.size();
}

bool rankSucceeds(Card c1, Card c2) {
	string rank1 = c1.getRank();
	string rank2 = c2.getRank();

	if (rank2 == "A") return (rank1 == "2");
	if (rank2 == "2") return (rank1 == "3");
	if (rank2 == "3") return (rank1 == "4");
	if (rank2 == "4") return (rank1 == "5");
	if (rank2 == "5") return (rank1 == "6");
	if (rank2 == "6") return (rank1 == "7");
	if (rank2 == "7") return (rank1 == "8");
	if (rank2 == "8") return (rank1 == "9");
	if (rank2 == "9") return (rank1 == "10");
	if (rank2 == "10") return (rank1 == "J");
	if (rank2 == "J") return (rank1 == "Q");
	if (rank2 == "Q") return (rank1 == "K");
	return false;
}

int main() {  
	stack<Card> tempDeck;
	tempDeck.push(Card("10", "Clubs"));
	tempDeck.push(Card("2", "Hearts"));
	tempDeck.push(Card("2", "Spades"));
	tempDeck.push(Card("6", "Diamonds"));
	tempDeck.push(Card("K", "Clubs"));
	tempDeck.push(Card("5", "Spades"));
	tempDeck.push(Card("6", "Clubs"));
	tempDeck.push(Card("4", "Hearts"));
	tempDeck.push(Card("3", "Clubs"));
	tempDeck.push(Card("7", "Hearts"));
	tempDeck.push(Card("4", "Clubs"));
	tempDeck.push(Card("J", "Clubs"));
	tempDeck.push(Card("7", "Spades"));
	tempDeck.push(Card("K", "Spades"));
	tempDeck.push(Card("2", "Diamonds"));
	tempDeck.push(Card("6", "Spades"));
	tempDeck.push(Card("4", "Diamonds"));
	tempDeck.push(Card("8", "Clubs"));
	tempDeck.push(Card("Q", "Hearts"));
	tempDeck.push(Card("J", "Hearts"));
	tempDeck.push(Card("8", "Spades"));
	tempDeck.push(Card("3", "Diamonds"));
	tempDeck.push(Card("4", "Spades"));
	tempDeck.push(Card("K", "Hearts"));

	stack<Card> deck;
	while (!tempDeck.empty()) {
		deck.push(tempDeck.top());
		tempDeck.pop();
	}

	vector<stack<Card>> tableaux_visible;
	for (int i = 0; i < 7; i++)
		tableaux_visible.push_back(stack<Card>());

	tableaux_visible[0].push(Card("Q", "Clubs"));
	tableaux_visible[1].push(Card("3", "Spades"));
	tableaux_visible[2].push(Card("5", "Diamonds"));
	tableaux_visible[3].push(Card("K", "Diamonds"));
	tableaux_visible[4].push(Card("7", "Diamonds"));
	tableaux_visible[5].push(Card("3", "Hearts"));
	tableaux_visible[6].push(Card("9", "Clubs"));

	vector<stack<Card>> tableaux_hidden;
	for (int i = 0; i < 7; i++)
		tableaux_hidden.push_back(stack<Card>());

	tableaux_hidden[1].push(Card("9", "Diamonds"));

	tableaux_hidden[2].push(Card("7", "Clubs"));
	tableaux_hidden[2].push(Card("8", "Hearts"));
	
	tableaux_hidden[3].push(Card("6", "Hearts"));
	tableaux_hidden[3].push(Card("Q", "Spades"));
	tableaux_hidden[3].push(Card("5", "Clubs"));
	
	tableaux_hidden[4].push(Card("A", "Clubs"));
	tableaux_hidden[4].push(Card("A", "Diamonds"));
	tableaux_hidden[4].push(Card("5", "Hearts"));
	tableaux_hidden[4].push(Card("9", "Spades"));
	
	tableaux_hidden[5].push(Card("10", "Diamonds"));
	tableaux_hidden[5].push(Card("J", "Diamonds"));
	tableaux_hidden[5].push(Card("8", "Diamonds"));
	tableaux_hidden[5].push(Card("A", "Hearts"));
	tableaux_hidden[5].push(Card("10", "Hearts"));
	
	tableaux_hidden[6].push(Card("A", "Spades"));
	tableaux_hidden[6].push(Card("10", "Spades"));
	tableaux_hidden[6].push(Card("Q", "Diamonds"));
	tableaux_hidden[6].push(Card("2", "Clubs"));
	tableaux_hidden[6].push(Card("J", "Spades"));
	tableaux_hidden[6].push(Card("9", "Hearts"));

	State initialState(deck, tableaux_visible, tableaux_hidden);
	initialState.fVal = initialState.gVal + initialState.hVal();

	priority_queue<State> fringe;
	
	cout << "Initial fVal: " << initialState.fVal << endl;

	fringe.push(initialState);

	vector<string> bestMoves;

	while (!fringe.empty())
	{
		State curState = fringe.top();
		fringe.pop();
		cout << "Current fVal: " << curState.fVal << endl;
		cout << "DeckSize: " << curState.deckSize() << ", PileSize: " << curState.pileSize() << endl;
		cout << "Cards remaining: " << curState.cardsRemaining << endl << endl;

		// Check if in win state
		if (curState.win())
		{
			bestMoves = curState.moveList;
			break;
		}

		// Action: Draw a card from the deck
		if (curState.deckSize() + curState.pileSize() > 0)
		{
			cout << "Drawing a card from the deck..." << endl;
			State newState(curState);
			// Add the # of frames it takes to move the cursor to the deck
			newState.gVal += DISTANCE[newState.position][0];

			// Deck is empty
			if (curState.deckSize() == 0) {
				newState.gVal += 7;
				newState.restack();
			}
			// Deck is not empty
			else
			{
				// Add the # of frames it takes to do the card move to the pile
				switch (newState.pileSize())
				{
				case 0:
					newState.gVal += 7;
					break;
				case 1:
					newState.gVal += 8;
					break;
				default:
					newState.gVal += 9;
				}
				// Move the next card to the pile
				newState.drawCard();
			}
			newState.position = 1;
			newState.moveList.push_back("DRAW");
			newState.fVal = newState.gVal + newState.hVal();

			fringe.push(newState);
		}

		// Action: Move a card from the pile to the foundation
		if (curState.pileSize() > 0)
		{
			Card topCard = curState.topPile();
			for (int i = 0; i < 4; i++)
			{
				if (topCard.getRank() == "A" && curState.fndEmpty(i))
				{
					cout << "Moving Ace from pile to foundation..." << endl;
					State newState(curState);
					newState.pileToFoundation(i);
					
					// Add the # of frames it takes to move the cursor to the pile
					newState.gVal += DISTANCE[newState.position][1];
					// Add the # of frames it takes to select a card
					newState.gVal += 2;
					// Add the # of frames it takes to move the cursor to the foundation
					newState.gVal += DISTANCE[1][2 + i];
					// Add the # of frames it takes to do the move
					newState.gVal += 20;
					if (newState.pileSize() >= 3)
						newState.gVal += 4;

					newState.position = 2 + i;
					newState.moveList.push_back("P->F" + i);
					newState.cardsRemaining--;
					newState.fVal = newState.gVal + newState.hVal();

					fringe.push(newState);
				}
				else if (!curState.fndEmpty(i))
				{
					cout << "Moving card from pile to foundation..." << endl;
					Card topFnd = curState.topFnd(i);
					if (topCard.getSuit() == topFnd.getSuit() && rankSucceeds(topCard, topFnd))
					{
						State newState(curState);
						newState.pileToFoundation(i);
						
						// Add the # of frames it takes to move the cursor to the pile
						newState.gVal += DISTANCE[newState.position][1];
						// Add the # of frames it takes to select a card
						newState.gVal += 2;
						// Add the # of frames it takes to move the cursor to the foundation
						newState.gVal += DISTANCE[1][2 + i];
						// Add the # of frames it takes to do the move
						newState.gVal += 20;
						if (newState.pileSize() >= 3)
							newState.gVal += 4;

						newState.position = 2 + i;
						newState.moveList.push_back("P->F" + i);
						newState.cardsRemaining--;
						newState.fVal = newState.gVal + newState.hVal();

						fringe.push(newState);
					}
				}
			}
		}

		// Action: Move a card from the pile to a tableau
		if (curState.pileSize() > 0)
		{
			Card topCard = curState.topPile();
			for (int i = 0; i < 7; i++)
			{
				if (curState.tabEmpty(i))
				{
					if (topCard.getRank() == "K")
					{
						cout << "Moving King from pile to Tableau #" << i << "..." << endl;
						State newState(curState);
						newState.pileToTableau(i);

						// Add the # of frames it takes to move the cursor to the pile
						newState.gVal += DISTANCE[newState.position][1];
						// Add the # of frames it takes to select a card
						newState.gVal += 2;
						// Add the # of frames it takes to move the cursor to the tableau
						newState.gVal += DISTANCE[1][6 + i];
						// Add the # of frames it takes to do the move
						newState.gVal += 20;
						if (newState.pileSize() >= 3)
							newState.gVal += 4;

						newState.position = 6 + i;
						newState.moveList.push_back("P->T" + i);
						newState.fVal = newState.gVal + newState.hVal();

						fringe.push(newState);
					}
				}
				else
				{
					Card tabCard = curState.topTab(i);
					if (goesOnTop(topCard, tabCard))
					{
						cout << "Moving card from pile to Tableau #" << i << "..." << endl;
						State newState(curState);
						newState.pileToTableau(i);

						// Add the # of frames it takes to move the cursor to the pile
						newState.gVal += DISTANCE[newState.position][1];
						// Add the # of frames it takes to select a card
						newState.gVal += 2;
						// Add the # of frames it takes to move the cursor to the tableau
						newState.gVal += DISTANCE[1][6 + i];
						// Add the # of frames it takes to do the move
						newState.gVal += 20;
						if (newState.pileSize() >= 3)
							newState.gVal += 4;

						newState.position = 6 + i;
						newState.moveList.push_back("P->T" + i);
						newState.fVal = newState.gVal + newState.hVal();

						fringe.push(newState);

					}
				}
					
			}
		}

		// Action: Move a card from a tableau to the foundation
		for (int i = 0; i < 7; i++)
		{
			if (!curState.tabEmpty(i))
			{
				Card topCard = curState.topTab(i);
				for (int j = 0; j < 4; j++)
				{
					if (curState.fndEmpty(j))
					{
						if (topCard.getRank() == "A")
						{
							cout << "Moving Ace from Tableau #" << i << " to Foundation #" << j << endl;
							State newState(curState);
							newState.tabToFoundation(i, j);
							// Add the # of frames it takes to move the cursor to the tableau
							newState.gVal += DISTANCE[newState.position][6 + i];
							// Add the # of frames it takes to select a card
							newState.gVal += 2;
							// Add the # of frames it takes to move the cursor to the foundation
							newState.gVal += DISTANCE[6 + i][2 + j];
							// Add the # of frames it takes to do the move
							newState.gVal += 20;

							newState.position = 2 + j;
							newState.cardsRemaining--;
							newState.moveList.push_back("T->F" + j);
							newState.fVal = newState.gVal + newState.hVal();

							fringe.push(newState);

						}
					}
					else {
						Card fndCard = curState.topFnd(j);
						if (goesOnTop(topCard, fndCard))
						{
							cout << "Moving card from Tableau #" << i << " to Foundation #" << j << endl;
							State newState(curState);
							newState.tabToFoundation(i, j);
							// Add the # of frames it takes to move the cursor to the tableau
							newState.gVal += DISTANCE[newState.position][6 + i];
							// Add the # of frames it takes to select a card
							newState.gVal += 2;
							// Add the # of frames it takes to move the cursor to the foundation
							newState.gVal += DISTANCE[6 + i][2 + j];
							// Add the # of frames it takes to do the move
							newState.gVal += 24;

							newState.position = 2 + j;
							newState.cardsRemaining--;
							newState.moveList.push_back("T->F" + j);
							newState.fVal = newState.gVal + newState.hVal();

							fringe.push(newState);
						}
					}
				}
			}
		}

		// Action: Move a card from a tableau to a tableau
		for (int i = 0; i < 7; i++)
		{
			if (!curState.tabEmpty(i))
			{
				Card topCard = curState.topTab(i);

				for (int j = 0; j < 7; j++)
				{
					if (i != j)
					{
						if (curState.tabEmpty(j))
						{
							if (topCard.getRank() == "K")
							{
								cout << "Moving King from Tableau #" << i << "to Tableau #" << j << endl;

								State newState(curState);
								newState.tabToTab(i, j, 1);
								// Add the # of frames it takes to move the cursor to the first tableau
								newState.gVal += DISTANCE[newState.position][6 + i];
								// Add the # of frames it takes to select a card
								newState.gVal += 2;
								// Add the # of frames it takes to move the cursor to the second tableau
								newState.gVal += DISTANCE[6 + i][6 + j];
								// Add the # of frames it takes to do the move
								newState.gVal += 22;

								newState.position = 6 + j;
								newState.moveList.push_back("T" + to_string(i) + "->T" + to_string(j));
								newState.fVal = newState.gVal + newState.hVal();

								fringe.push(newState);
							}
						}
						else
						{
							int num = curState.moveNum(i, j);
							if (num > 0)
							{
								cout << "Moving " << num << " cards from Tableau #" << i << " to Tableau #" << j << endl;

								State newState(curState);
								newState.tabToTab(i, j, num);
								// Add the # of frames it takes to move the cursor to the first tableau
								newState.gVal += DISTANCE[newState.position][6 + i];
								// Add the # of frames it takes to select a card
								newState.gVal += 2;
								// Add the # of frames it takes to move the cursor to the second tableau
								newState.gVal += DISTANCE[6 + i][6 + j];
								// Add the # of frames it takes to do the move
								newState.gVal += 11 * (num + 1);

								newState.position = 6 + j;
								newState.moveList.push_back("T" + to_string(i) + "->T" + to_string(j));
								newState.fVal = newState.gVal + newState.hVal();

								fringe.push(newState);
							}
						}
					}
				}
			}
		}

		cout << endl;
	}

	for (int i = 0; i < bestMoves.size(); i++)
	{
		cout << bestMoves[i] << " ";
	}
	cout << endl;

	return 1;
}