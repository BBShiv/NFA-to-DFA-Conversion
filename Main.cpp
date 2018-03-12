#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <utility>
#include <algorithm>
using namespace std;

/*************************************************************
* Global Variables, Structs, Typedefs
**************************************************************/

struct DFAState {
	bool marked;
	vector<int> states;
	map<char, int> moves;
};

typedef map<int, DFAState> DFATableType;
typedef map<int, map<char, vector<int>>> NFATableType;

int INIT_STATE, TOTAL_STATES;
vector<int> FINAL_STATES;
vector<char> ALPHABET;
NFATableType STATE_TABLE;
DFATableType DFA_STATE_TABLE;

/*************************************************************
* Membership Function:
* Check if a vector<int> contains a certain integer.
**************************************************************/
bool doesVectorContain(vector<int> vec, int key) {
	for (vector<int>::const_iterator k = vec.begin(); k != vec.end(); k++) {
		if (*k == key) {
			return true;
		}
	}
	return false;
}

/*************************************************************
* Print Vector Function:
* Print a given vector in e-closure format: {x,y,z,..}
**************************************************************/
void printVector(vector<int> vec) {
	cout << "{";
	for (vector<int>::const_iterator i = vec.begin(); i != vec.end(); i++) {
		if (i != vec.end() - 1) {
			cout << *i << ",";
		}
		else {
			cout << *i;
		}
	}
	cout << "} ";
}

/*************************************************************
* isAnythingUnmarked function:
* Loop over the DTransition table and look for unmarked states.
* If an unmarked state exists, return the int corresponding
* to that state. If all states are marked, return -1.
**************************************************************/
int isAnythingUnmarked(DFATableType DFATable) {
	int size = DFATable.size();
	for (int i = 0; i < size; i++) {
		DFAState current = DFATable[i];
		if (!(current.marked)) {
			return i;
		}
	}
	//everything is marked
	return -1;
}

/*************************************************************
* isAMember function:
* Checks to see if a vector<int> representing a DFAState
* is already contained within the DFAStateTable.
* If it is a member, return the state index. If not, return -1.
**************************************************************/
int isAMember(vector<int> state, DFATableType DFATable) {
	int size = DFATable.size();
	for (int i = 0; i < size; i++) {
		DFAState current = DFATable[i];
		//compare the two vectors
		if (current.states == state) {
			return i;
		}
	}
	//state is not already within the table
	return -1;
}


/*************************************************************
* Move Function
*
* Given a state T and a move x, return all reachable states
* given that moves as a vector<int>.
**************************************************************/
vector<int> move(vector<int> T, char move, NFATableType NFATable) {
	vector<int> ans;
	for (vector<int>::const_iterator j = T.begin(); j != T.end(); j++) {
		//get the moves vector for the state and move
		vector<int> reachableStates = NFATable[*j][move];
		for (vector<int>::const_iterator k = reachableStates.begin(); k != reachableStates.end(); k++) {
			//if k is not already in ans, add it
			if (!(doesVectorContain(ans, *k))) {
				ans.push_back(*k);
			}
		}
	}
	sort(ans.begin(), ans.end());
	return ans;
}

/*************************************************************
* DFAState Constuctor Function
*
* Helper function to initialize and return a new DFAState.
**************************************************************/
DFAState newDFAState(bool mark, vector<int> s) {
	DFAState newState;
	map<char, int> init;
	newState.marked = mark;
	newState.states = s;
	newState.moves = init;
	return newState;
}

/*************************************************************
* findFinalDFAStates Function
*
* Takes in the final state(s) of the NFA, parses the DFATable
* to see which states should be marked as final.
**************************************************************/
vector<int> findFinalDFAStates(DFATableType DFATable, vector<int> finalStates) {
	vector<int> finals;
	for (int i = 0; i < DFATable.size(); i++) {
		for (vector<int>::const_iterator k = finalStates.begin(); k != finalStates.end(); k++) {
			if (doesVectorContain(DFATable[i].states, *k)) {
				finals.push_back(i);
			}
		}
	}
	return finals;
}

/*************************************************************
* Subset Construction Function
*
* Create the corresponding DFA Transition Table given an
* initial state, final state(s), and an NFA Table
**************************************************************/
void subsetConstruction(int initialState, vector<int> finalStates, NFATableType &NFATable, DFATableType &DFATable) {

	int currentDFAStateNumber = 0;

	vector<int> initialStateVector;
	initialStateVector.push_back(initialState);
	DFAState initState = newDFAState(false, initialStateVector);
	
	DFATable[currentDFAStateNumber] = initState;
	currentDFAStateNumber++;

	while (isAnythingUnmarked(DFATable) >= 0) {

		int k = isAnythingUnmarked(DFATable);
		DFATable[k].marked = true;
		cout << "\nMark " << k << endl;

		for (vector<char>::const_iterator w = ALPHABET.begin(); w != ALPHABET.end(); w++) {

			vector<int> theMove = move(DFATable[k].states, *w, NFATable);

			//pretty print the move if it is not empty
			if (!(theMove.empty())) {
				printVector(DFATable[k].states);
				cout << "--" << *w << "--> ";
				printVector(theMove);
				cout << "\n";
			}

			int j = isAMember(theMove, DFATable);

			if (j >= 0) {
				cout << j << "\n";
				DFATable[k].moves[*w] = j;
			}
			else {
				if (!(theMove.empty())) {

					cout << currentDFAStateNumber << "\n";

					//add alphamove as a new state to the DFATable.
					DFAState newState = newDFAState(false, theMove);
					DFATable[currentDFAStateNumber] = newState;
					DFATable[k].moves[*w] = currentDFAStateNumber;
					currentDFAStateNumber++;
				}
				else {
					DFATable[k].moves[*w] = -1;
				}
			}
		}
	}//end while
	cout << "\n";
}

/*************************************************************
* ReadFile function
*
* handles parsing input file and initializing all
* high level variables.
**************************************************************/
void readFile(string filename) {

	string line;
	ifstream myfile(filename);

	if (myfile.is_open()) {
		istringstream iss(line);
		char test = iss.get();

		/*************************************
		* GET TOTAL STATES
		*************************************/
		getline(myfile, line);
		TOTAL_STATES = atoi(line.c_str());

		/*************************************
		* GET ALPHABET
		*************************************/
		getline(myfile, line);
		string trash;
		char move;
		istringstream alphabet(line);

		while (alphabet >> move) {
			ALPHABET.push_back(move);
		}

		/*************************************
		* GET FINAL STATES
		*************************************/
		getline(myfile, line);
		char finalState;

		for (int i = 0; i < line.size(); i++) {
			if (line[i] != ' ') {
				finalState = line[i] - '0';
				FINAL_STATES.push_back(finalState);
			}
		}

		/*************************************
		* GET INITIAL STATE
		*************************************/

		getline(myfile, line);
		INIT_STATE = atoi(line.c_str());

		/*************************************
		* INITIALIZE NFA STATES
		*************************************/
		map<char, vector<int> > StateMovesMap;
		vector<int> statesA, statesB;
		int j = 0, previousState = NULL;
		getline(myfile, line);				//get first line of transitions set in file
		
		while (!myfile.eof()) {
			char alphabet;
			int startState, transitionState;
			
			istringstream _iss(line);									
			_iss >> startState;

			if (j == 0) {
				previousState = startState;			//initially set previousState to the first state of the transitions set in file
			}

			if (previousState != startState) {
				//all moves for given state is formatted, move to STATE_TABLE
				sort(statesA.begin(), statesA.end());
				sort(statesB.begin(), statesB.end());
				StateMovesMap[ALPHABET[0]] = statesA;
				StateMovesMap[ALPHABET[1]] = statesB;
				STATE_TABLE[previousState] = StateMovesMap;

				map<char, vector<int>> StateMovesMap;
				statesA.clear();
				statesB.clear();
				previousState = startState;
			}

			_iss.ignore();
			_iss >> alphabet;
			_iss.ignore();
			_iss >> transitionState;

			if (alphabet == 'a') {
				statesA.push_back(transitionState);
			}
			else {
				statesB.push_back(transitionState);
			}
			j += 1;
			getline(myfile, line);

			if (line == "") {
				sort(statesA.begin(), statesA.end());
				sort(statesB.begin(), statesB.end());
				StateMovesMap[ALPHABET[0]] = statesA;
				StateMovesMap[ALPHABET[1]] = statesB;
				STATE_TABLE[previousState] = StateMovesMap;
			}
		}
	}
}

/*************************************************************
* printFile function
*
* handles parsing input file and initializing all
* high level variables.
**************************************************************/
void printFile(DFATableType DFATable) {
	ofstream myFile;

	myFile.open("DFA.txt");

	while (myFile.is_open()) {
		myFile << TOTAL_STATES << endl;

		for (vector<char>::const_iterator k = ALPHABET.begin(); k != ALPHABET.end(); k++) {
			myFile << *k;
		}
		myFile << endl;

		for (vector<int>::const_iterator k = FINAL_STATES.begin(); k != FINAL_STATES.end(); k++) {
			myFile << *k << " ";
		}
		myFile << endl;
		myFile << INIT_STATE << endl;

		for (int i = 0; i < DFATable.size(); i++) {	
			for (vector<char>::const_iterator k = ALPHABET.begin(); k != ALPHABET.end(); k++) {
				if (DFATable[i].moves[*k] != -1) {
					myFile << i << " ";
					myFile << *k << " ";
					myFile << DFATable[i].moves[*k] << endl;
				}
			}
		}
		myFile.close();
	}
}

/*************************************************************
* Main function
*
* Parse file, subset construction, print output.
**************************************************************/
int main(int argc, char** argv) {

	//parse input file to populate the global variables
	string filename;
	cout << "Enter in the name of the file with the NFA contents: ";
	cin >> filename;
	readFile(filename);

	//subset construction algorithm
	subsetConstruction(INIT_STATE, FINAL_STATES, STATE_TABLE, DFA_STATE_TABLE);

	printFile(DFA_STATE_TABLE);

	return 0;
}