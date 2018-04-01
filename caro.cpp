/*
 * caro.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <thread>         // std::thread
#include <mutex>          // std::mutex

using namespace std;
using namespace std::chrono;

//#define VERBOSE3 1
//#define DEBUGSCORING 1
#ifdef VERBOSE3
#define VERBOSE2 1
#endif

#ifdef VERBOSE2
#define VERBOSE 1
#endif

//#define VERBOSE2 1
#define VERBOSE0 1
#define HASH 1
#define MAGICNUMBER 0x000BEE00
#define SIGNUM 0x000BABE0
#define NUM9 NUM8*2
#define NUM8 NUM7*2
#define NUM7 NUM6*3
#define NUM6 NUM5*3
#define NUM5 NUM4*3
#define NUM4 NUM3*3
#define NUM3 NUM2*3
#define NUM2 6
#define NUM1 2

#define PRINTSCORE 1
#define prompt(a) {cout << a << endl; cin.get(); }
// (a+b)/2 =50 percent -> a+b = 2*p
#define fscale(a,b,p)  a+((b-a)*p)/100
#define runtimeInMicroSecond 9000000*1 // 9 seconds
#include "caro.h"
int search_depth = 5;
int search_width = 5;
int setBreastDepth[][2] = { { 3, 14 }, { 5, 8 }, { 10, 6 }, { 15, 4 } };
int setWidthDepth[][2] = { { 14, 6 }, { 10, 8 }, { 6, 8 }, { 6, 8 }, { 4, 10 } };
int maxSearchDepth = 14;
#define MINSCORE 4
int adjustCntArray[] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
		100, 100, 100, 100 };
int bitcntArray[] = { 0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM6, NUM6,
NUM6, NUM6 };
int bitcntArrayscale[] = { 0, fscale(0, NUM1, 50), NUM1, fscale(NUM1, NUM2, 50),
NUM2, fscale(NUM2, NUM3, 50), NUM3, fscale(NUM3, NUM4, 50), NUM4, fscale(NUM4,
		NUM5, 50),
NUM5, fscale(NUM5, NUM6, 50), NUM6, NUM7, NUM8, NUM9, NUM9,
NUM6, NUM6 };

#define BIAS_PERCENT 100  // more than opponent
#define biasDefendAdjust(val) val*BIAS_PERCENT/100

#define dbestValue() {0x1EADBEEF,0,in_depth}
#define dworstValue() {0,0x1D0CDEAD,in_depth}
#define dbestMove() {0x1B1CBAD0,0x1BIGBAD0,depth}
#define dworstMove() {0,0,depth}
int gdebug = 0;
int debugScoring = 0;
int debugScoringd = 0;
int debugScoringAll = 0;
int docheck = 0;

int debugBestPath = 0;
int debugHash = 0;
int debugAI = 0;
int debugAIbest = 0, moreDepth = 0, training = 0;
int debugRow, debugCol;
int interactiveDebug;
int debugTrace = 0;
int underDebug = 0, lowerMin = 0, higherMin = 0;
int inspectCell;
bool debugCell;
unsigned char iC[2];
bool oneShot = true;
char globalInputStr[80];
char *gisptr;
getInputStr input;

tScore::tScore() {
	myScore = 0;
	oppScore = 0;
	ts_ret = {0,0};
	cellPtr = nullptr;
}

tScore::tScore(scoreElement a) {
	myScore = a.myScore;
	oppScore = a.oppScore;
	cellPtr = a.cellPtr;
	ts_ret = {0,0};
}

hashTable ahash;

void caro::reset() {
	moveCnt = 0;
	lastMoveIndex = 0;
	saveLast2p = 0;
	for (int row = 1; row < size; row++)
		for (int col = 1; col < size; col++)
			board[row][col].val = E_FAR;
}

caro::~caro() {
	// TODO Auto-generated destructor stub
}

/*
 * print out the entire table -- only for text base, new routine is needed for GUI
 */

void caro::clearScore() {
	for (int row = 0; row <= size; row++)
		for (int col = 0; col <= size; col++) {
			board[row][col].score.myScore = 0;
			board[row][col].score.oppScore = 0;
		}
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */

/*
 * extracting a line for the purpose of scoring
 * This will need to work with how scoring is done.  Extracting will need to go
 * in hand.  So, this will need to change depend on the method
 * ONGOING CODING -- NOT DONE
 */
#ifdef ROWCOL
Line caro::extractLine(int dir, int in_row, int in_col, bool &ending,
		bool debugThis) {
	Line aline;
	int bitcnt = 0;
	aline.val = 0;
	aline.cnt = 0;
	int unconnected = 0;
	ending = false;
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	rowCol crDir;
	rowCol in(in_row, in_col);
	rowCol crCurrCell(in_row, in_col);
	rowCol crOriCell = crCurrCell;
	rowCol crBackoffCell;
// reverse the row to match up with old scheme
	crDir.setDirection(dir);
	val = cellVal(in);

	oppval = oppositeVal(val);
	//debugLine = true;

	crCurrCell = in;

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool isVal = false;
	aline.type = val;
	int freeEnd = 0;

	if (debugCell) {
		cout << "Dir=" << dir << " aiPlay=" << my_AI_Play << " val=" << val;
		cout << endl << board[in_row][in_col] << " --> ";
	}
	int spaced = 0;
	isVal = false;
	for (int i = 0; i < SEARCH_DISTANCE; i++) {
		if (cellVal(crCurrCell)&oppval
		) {
			if (isVal & (cellVal(crCurrCell) == oppval))
			aline.blocked = 1;
			break;
		} else if (cellVal(crCurrCell) == val) {
			crBackoffCell = crCurrCell;
		} else {
			if (spaced++ > 1) {
				freeEnd++;
				break;
			}
		}
		isVal = cellVal(crCurrCell)== val;
		crCurrCell.moveToDir(crDir);
	}
	//prompt("hit Enter q");
	if (aline.blocked || (spaced > 2)) {
		crCurrCell = crBackoffCell;
	} else {
		// Did not find O_, switch back to ori and scan in reverse
		crCurrCell = crOriCell;
		crDir.reverse();
		if (debugCell) {
			cout << endl << board[in_row][in_col] << " <-- ";
		}
		int spaced = 0;
		isVal = false;
		for (int i = 0; i < SEARCH_DISTANCE; i++) {
			if (cellVal(crCurrCell)&oppval
			) {
				if (isVal & (cellVal(crCurrCell) == oppval))
				aline.blocked = 1;
				break;
			} else if (cellVal(crCurrCell) == val) {
				crBackoffCell = crCurrCell;
			} else {
				if (spaced++ > 1) {
					freeEnd++;
					break;
				}
			}
			isVal = cellVal(crCurrCell)== val;
			crCurrCell.moveToDir(crDir);
		}
		//	prompt("hit Enter q2");

		crCurrCell = crBackoffCell;
	}

	// Now reverse direction
	crDir.reverse();

	aline.val = 0;
	aline.connected = 0;
	aline.offset = 0;
	int save = 0;
	if (debugCell) {
		cout << endl << board[in_row][in_col] << " --> ";
	}
	int prev_unconnected = 0;
	int continuous = 0;
	aline.continuous = 0;
	ending = false;
	for (int i = 0; i < (SEARCH_DISTANCE); i++) {
		aline.val = aline.val << 1;
		aline.cnt++;
		isVal = cellVal(crCurrCell)== val;
		if (cellVal(crCurrCell)== val) {
			if ((unconnected == 1)) { // double check this
				if (aline.continuous < 4) {
					continuous = aline.continuous;
					; // take just 1 bubble for less than 4, dont want XXXX_X
					aline.continuous = 0;
				}
				aline.connected = save + 1;
				//	if (aline.connected < 3)
				prev_unconnected++;
			} else
			aline.connected++;
			continuous++;
			if ((continuous >= 5) && (prev_unconnected == 0)) //&& (val == currPlay))
			ending = true;// need to correct true ending on upper level, where u know about
						  //current Play, i.e. block-abled or game over
			bitcnt++;
			aline.val = aline.val | 0x1;
			unconnected = 0;
		} else {
			if (aline.continuous == 0)
			aline.continuous = continuous;
			continuous = 0;
			if (aline.connected > save)
			save = aline.connected;
			aline.connected = 0;
			if (unconnected++ > 2)
			break;
		}
		crCurrCell.moveToDir(crDir);
		if (cellVal(crCurrCell)&oppval) {
			if (isVal & (cellVal(crCurrCell) == oppval))
			aline.blocked++;
			break;
		}
	}
	if (prev_unconnected > 1)
		aline.continuous--;

	if (aline.continuous == 0)
		aline.continuous = continuous;
	aline.cnt += freeEnd * 2;

	if (save > aline.connected)
		aline.connected = save;
	/*
	 if ((aline.connected > 2) && (prev_unconnected >= 1)) {
	 aline.connected -= prev_unconnected - 1;
	 }
	 */
	if (aline.connected == 1)
		aline.connected = 0;
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt < 5)
		aline.connected = 0;
	if (aline.connected < 4) {
		aline.offset -= prev_unconnected;
	}
	aline.connected = (aline.connected * 2);

	if (aline.blocked) {
		if (aline.connected <= 8)
			aline.offset -= 2;
	}
	if ((val == my_AI_Play) && (aline.connected >= 6))
		aline.offset++;

	/*
	 if ((val == aiPlay) && (aline.connected > 0))
	 aline.connected--; // go first, assume that opponent to block  (1 less)
	 */
	if (debugCell) {
		cout << endl;
		cout << aline;
	}
	return aline;
}
#else
Line caro::extractLine(int dir, int row, int col, bool &ending,
		bool debugThis) {
	Line aline;
	int bitcnt = 0;
	aline.val = 0;
	aline.cnt = 0;
	int unconnected = 0;
	ending = false;
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	val = board[row][col].val;
	bool debugLine = debugThis
	&& (debugScoringAll || debugScoring
			|| cdbg.ifMatch(&board[row][col], -1, 1));
	oppval = oppositeVal(board[row][col].val);
	//debugLine = true;
	cell *currCell = &board[row][col];
	cell *oriCell = currCell;
	cell *backoffCell;//, *lastone;

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool prevVal = false;
	aline.type = val;
	int freeEnd = 0;
	if (debugLine) {
		cout << "Dir=" << dir << " aiPlay=" << my_AI_Play << " val=" << val;
		cout << endl << board[row][col] << " --> ";
	}
//	lastone = currCell;
	int spaced = 0;
	prevVal = false;
	for (int i = 0; i < SEARCH_DISTANCE; i++) {
		if (currCell->val & oppval) {
			if (prevVal & (currCell->val == oppval))
			aline.blocked = 1;
			break;
		} else if (currCell->val == val) {
			backoffCell = currCell;
		} else {
			if (spaced++ > 1) {
				freeEnd++;
				break;
			}
		}
		prevVal = currCell->val == val;

		currCell = currCell->near_ptr[dir];
	}
	//prompt("hit Enter q");
	if (aline.blocked || (spaced > 2)) {
		currCell = backoffCell;
	} else {
		// Did not find O_, switch back to ori and scan in reverse
		currCell = oriCell;
		dir = ReverseDirection(dir);
		if (debugLine) {
			cout << endl << board[row][col] << " <-- ";
		}
		int spaced = 0;
		prevVal = false;
		for (int i = 0; i < SEARCH_DISTANCE; i++) {
			if (currCell->val & oppval) {
				if (prevVal & (currCell->val == oppval))
				aline.blocked = 1;
				break;
			} else if (currCell->val == val) {
				backoffCell = currCell;
			} else {
				if (spaced++ > 1) {
					freeEnd++;
					break;
				}
			}
			prevVal = currCell->val == val;

			currCell = currCell->near_ptr[dir];
		}
		//	prompt("hit Enter q2");

		currCell = backoffCell;
	}

	// Now reverse direction
	dir = ReverseDirection(dir);

	aline.val = 0;
	aline.connected = 0;
	aline.offset = 0;
	int save = 0;
	if (debugLine) {
		cout << endl << board[row][col] << " --> ";
	}
	int prev_unconnected = 0;
	int continuous = 0;
	aline.continuous = 0;
	ending = false;
	for (int i = 0; i < (SEARCH_DISTANCE); i++) {
		if (debugLine)
		cout << *currCell << "-";
		aline.val = aline.val << 1;
		aline.cnt++;
		prevVal = currCell->val == val;
		if (currCell->val == val) {
			if ((unconnected == 1)) { // double check this
				if (aline.continuous < 4) {
					continuous = aline.continuous;
					; // take just 1 bubble for less than 4, dont want XXXX_X
					aline.continuous = 0;
				}
				aline.connected = save + 1;
				//	if (aline.connected < 3)
				prev_unconnected++;
			} else
			aline.connected++;
			continuous++;
			if ((continuous >= 5) && (prev_unconnected == 0)) //&& (val == currPlay))
			ending = true;// need to correct true ending on upper level, where u know about
						  //current Play, i.e. block-abled or game over
			bitcnt++;
			aline.val = aline.val | 0x1;
			unconnected = 0;
		} else {
			if (aline.continuous == 0)
			aline.continuous = continuous;
			continuous = 0;
			if (aline.connected > save)
			save = aline.connected;
			aline.connected = 0;
			if (unconnected++ > 2)
			break;
		}
		currCell = currCell->near_ptr[dir];
		if (currCell->val & oppval) {
			if (prevVal & (currCell->val == oppval))
			aline.blocked++;
			break;
		}
	}
	if (prev_unconnected > 1)
	aline.continuous--;

	if (aline.continuous == 0)
	aline.continuous = continuous;
	aline.cnt += freeEnd * 2;

	if (save > aline.connected)
	aline.connected = save;
	/*
	 if ((aline.connected > 2) && (prev_unconnected >= 1)) {
	 aline.connected -= prev_unconnected - 1;
	 }
	 */
	if (aline.connected == 1)
	aline.connected = 0;
	if (aline.cnt > 8)
	aline.cnt = 8;
	if (aline.cnt < 5)
	aline.connected = 0;
	if (aline.connected < 4) {
		aline.offset -= prev_unconnected;
	}
	aline.connected = (aline.connected * 2);

	if (aline.blocked) {
		if (aline.connected <= 8)
		aline.offset -= 2;
	}
	if ((val == my_AI_Play) && (aline.connected >= 6)) // Favor offensive
	aline.offset++;

	/*
	 if ((val == aiPlay) && (aline.connected > 0))
	 aline.connected--; // go first, assume that opponent to block  (1 less)
	 */
	if (debugLine) {
		cout << endl;
		cout << aline;
	}
	return aline;
}
#endif
int Line::evaluate(bool ending) {
// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
// self learning)
	score = 0;
	int bitcnt = 0;
	if (cnt < 5)
		score = 0;
	else if (ending) {
		score = MAGICNUMBER; // MAGICNUMBER is not terminating, unless with ownplay
	} else {
		int tval = val;
		while (tval) {
			bitcnt = bitcnt + (tval & 1); // recalculate bitcnt
			tval = tval >> 1;
		}
		score = bitcntArrayscale[connected + offset];
	}
#ifdef HASH
	ahash.addEntry(val, connected / 2, cnt, score);
#endif
	return score;
}

void FourLines::print() {
	for (int dir = East; dir < West; dir++) {
		Xlines[dir].print();
	}
}

aScore caro::score1Cell(const int currPlay, const int row, const int col,
		bool debugThis) {
	FourLines astar;
	bool saveCheck = false;
	bool redothisone = false;
	aScore testSaveScore = { 0, 0, 0 };
	if (board[row][col].val & E_CAL) {
		// calculating overall score
		return board[row][col].score;

		saveCheck = true;
		testSaveScore = board[row][col].score;
	}
	aScore rtn;
	rtn.connectedOrCost = 0;

	do {

		points scores[2] = { 0, 0 };
		int opnVal = oppositeVal(my_AI_Play);
		int tempVal[2];
		bool ending;
		tempVal[0] = my_AI_Play;
		tempVal[1] = opnVal;
		//	saveScoreVal(row, col, saveScores, saveVals);
		int saveVal = board[row][col].val;
		for (int j = 0; j < 2; j++) {
			int chk = 0;

			int curVal = tempVal[j];
			int ill_6, ill_4, ill_3;
			ill_6 = ill_4 = ill_3 = 0;
			setCell(curVal, row, col, E_FAR);
			Line compareLine;
			for (int dir = East; dir < West; dir++) {
				astar.Xlines[dir] = extractLine(dir, row, col, ending,
						debugThis);
				points tscore = astar.Xlines[dir].evaluate(ending);
				/*
				 compareLine = copyLine(dir, row, col, ending, debugThis);
				 compareLine.evaluate(ending);

				 if (!(compareLine == astar.Xlines[dir])) {
				 cout << "Found DIFF" << ends;
				 char ach;
				 cin >> ach;
				 }
				 */
				if (astar.Xlines[dir].continuous >= (6)) {
					ill_6 = 1;
				} else if (astar.Xlines[dir].blocked == 0) {
					if (astar.Xlines[dir].connected == (3 * 2)) {
						ill_3++;
					} else if (astar.Xlines[dir].connected == (4 * 2)) {
						ill_4++;
					}
				}
				if ((astar.Xlines[dir].cnt > 4)
						|| (astar.Xlines[dir].blocked < 2)) {
					if (astar.Xlines[dir].continuous >= 5) {
						chk = 4;
						//	cout << "HS<-dir=" << dir << " " << astar.Xlines[dir];
					} else if ((astar.Xlines[dir].continuous == 4)) {
						if (astar.Xlines[dir].blocked == 1)
							chk = 2; // is 3 but blocked on 1 side,
						else
							chk = 3;
						//cout << "HS<-dir=" << dir << " " << astar.Xlines[dir];
					} else if ((astar.Xlines[dir].continuous == 3)) {
						if (astar.Xlines[dir].blocked == 0)
							chk = 1;
					}
				}
				if ((tscore > NUM1) && (scores[j] > NUM1)) {
					if (tscore < scores[j])
						scores[j] = scores[j] * 2 + tscore * 1;
					else
						scores[j] = scores[j] * 1 + tscore * 2;

				} else
					scores[j] = scores[j] + tscore;
				scores[j] = MIN(scores[j], MAGICNUMBER); // not to overflow
				if (chk > abs(rtn.connectedOrCost))
					rtn.connectedOrCost = chk;
				if ((debugThis && (debugCell || debugScoring))) {
					cout << "CurrPlay=" << currPlay << " "
							<< convertToChar(curVal) << "_ dir=" << dir
							<< " ctnuos=" << astar.Xlines[dir].continuous
							<< " connected=" << astar.Xlines[dir].connected
							<< " blk=" << astar.Xlines[dir].blocked << " cnt="
							<< astar.Xlines[dir].cnt << "-chk" << chk
							<< "-rtn.c" << rtn.connectedOrCost << "-end"
							<< ending << "-score=" << scores[j] << "," << tscore
							<< " -ill" << ill_6 << ill_4 << ill_3 << endl;

				}
			}

			if ((ill_6 || (ill_4 > 1) || (ill_3 > 1)) && (curVal == X_)) { // curVal, not currPlay
				scores[j] = 0; //-0x200; TODO
				rtn.connectedOrCost = 0;
			} else if (chk > abs(rtn.connectedOrCost)) {
				rtn.connectedOrCost = chk;
			}
			if (curVal == my_AI_Play)
				rtn.connectedOrCost = -rtn.connectedOrCost; // neg for O_ -- cheezy

			// doing this here intead of after for loop bc of how setcell works
			board[row][col].val = saveVal;
		}

		rtn.myScore = scores[0];
		rtn.oppScore = scores[1];
		board[row][col].score = rtn;

		// only turn on CAL if NEAR or TNEAR
		if (abs(rtn.connectedOrCost) < 3)
			if (board[row][col].val & (E_NEAR | E_TNEAR)) { // redudant!
				board[row][col].val |= E_CAL; // will not be re-evaluate
				if ((row == 16) || (col == 16)) {
					cout << "col=" << col << " row=" << row << endl;
					char ach;
					cin >> ach;
				}
			}
		if ((debugThis && (debugCell || debugScoring))) {
			cout << board[row][col] << board[row][col].score << endl;
		}
		if (saveCheck) {
			if (redothisone) {
				debugScoringAll = 0;
				redothisone = false;
				break;
			}
			if (!(testSaveScore == rtn)) {
				cout << "SHORTCUT different, saveScore =" << testSaveScore
						<< " newScore =" << rtn;
				print(SYMBOLMODE2);
				redothisone = true;
				debugScoringAll = 1;
			}
		}
	} while (redothisone);
	return rtn;
}

/*
 *
 *
 *
 *
 *
 */
bool betterMove(scoreElement & a, scoreElement & b) {
	return ((a.myScore + a.oppScore) > (b.myScore + b.oppScore));
}
bool betterValue(scoreElement & a, scoreElement & b) {
	return ((a.myScore - a.oppScore) > (b.myScore - b.oppScore));
}
bool lessValue(scoreElement & a, scoreElement & b) {
	return ((a.myScore - a.oppScore) < (b.myScore - b.oppScore));
}
/*
 *
 *
 *
 */
void caro::modifyDebugFeatures(int debugId) {
	char ffn[80] = "savefile.txt";

	switch (debugId) {

	case -23:
		cout << "Turn " << FLIP(debugScoringd) << " debugScoringd" << endl;

		break;
	case -24:
		cout << "Turn " << FLIP(debugScoringAll) << " debugScoringAll" << endl;

		break;
	case -25:
		cout << "Turn " << FLIP(debugHash) << " debugHash" << endl;

		break;
	case -26:
		cout << "Turn " << FLIP(debugTrace) << " debugTrace" << endl;
		break;

	case -4:
		cout << "Turn " << FLIP(underDebug) << " underDebug" << endl;
	case -5:
		cout << "Turn " << FLIP(lowerMin) << " lowerMin" << endl;
	case -6:
		cout << "Turn " << FLIP(higherMin) << " higherMin" << endl;
	case -3:
		cout << "Turn " << FLIP(debugScoring) << " debugScoring" << endl;

		break;
	case -98:
		cout << "Turn " << FLIP(docheck) << " docheck" << endl;
		break;
	case -99:
		cout << "Turn " << FLIP(interactiveDebug) << " interactiveDebug"
				<< endl;
		break;
	case -13:

		for (int i = 1; i < 1000; i++) {
			sprintf(ffn, "savefile%d.txt", i);

			cout << "filename =" << ffn;
			ifstream ifile(ffn);
			if (ifile) {
				ifile.close();
				continue;
			}
			ofstream ofile(ffn);
			if (ofile) {
				char ans[80];
				cout << " Writing to " << ffn << " ?" << endl;
				cin >> ans;
				if ((ans[0] == 'n') || (ans[0] == 'N')) {
					ofile.close();
					continue;
				}
				ofile << *this;
				cout << *this;
				ofile.close();
				break;
			}
		}
	default:
		break;
	}
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
auto start = high_resolution_clock::now();

int caro::createNodeList(int thisNodeCnt, int in_NodeCnt,
		vector<scoreElement>& bestScoreArray, int& highestConnected,
		scoreElement& bestScore) {
	sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
	thisNodeCnt = MIN(in_NodeCnt, (int )bestScoreArray.size()); // size of bestScoreArray can be smaller than "width"
	// prunning -- necessary to remove bad moves
	if (highestConnected) {
		int size = 0;
		for (int i = 0; i < (int) (bestScoreArray.size()); i++) {
			//	cout << *bestScoreArray[i].cellPtr << " cnnt=" << bestScoreArray[i].connected << endl;
			if (bestScoreArray[i].bestMove() == 0)
				break;

			if ((abs(bestScoreArray[i].connectedOrCost) >= abs(highestConnected))
					|| ((highestConnected == 3)
							&& (bestScoreArray[i].connectedOrCost == -2))
					|| ((highestConnected == -3)
							&& (bestScoreArray[i].connectedOrCost == 2))) {
				bestScoreArray[size++] = bestScoreArray[i];
			}
		}
		//	if (size < thisNodeCnt)
		thisNodeCnt = size;

		highestConnected = 0;
	} else {
		int i;
		for (i = 0; i < (int) (bestScoreArray.size()); i++)
			if (bestScoreArray[i].bestMove() <= MINSCORE)
				break;
		thisNodeCnt = MIN(i, thisNodeCnt);

	}
	bestScore = bestScoreArray[0]; // BestScore
	return thisNodeCnt;
}

int caro::debugSetup(int thisNodeCnt, int in_depth, int currPlay, bool topLevel,
		bool debugThis, int terminated, vector<scoreElement>& bestScoreArray,
		debugid& debug, vector<scoreElement>& returnScoreArray) {
	char ach;
	int inputRemain = 0;
	do {
		for (int i = 0; i < thisNodeCnt; i++) {
			printf("\t<%d,%d>", in_depth, i);
			printf("%C", convertCellToStr(currPlay));
			cout << bestScoreArray[i] << "|";
			if (i % 3 == 0)
				cout << endl;
		}
		cout << endl;
		if (topLevel && !debugThis)
			break;
		if (debugThis && (terminated == 0)) {
			int n;
			int l, row, col;
			char inputStr[40], ccol;
			debug.id.clear();
			vector<scoreElement> saveList = bestScoreArray;
			int saveW = thisNodeCnt;
			do {
				n = 1;
				thisNodeCnt = saveW;
				bestScoreArray = saveList;
				//		printDebugInfo(0, 0, &currTrace);
				cout
						<< "Enter # for debug (e={new order}, a ={all, no debug}, i={in order, with debug on specify #}"
						<< " width=" << thisNodeCnt << endl;
				for (int size = 0; size <= thisNodeCnt;) {
					inputRemain = input.mygetstr(inputStr);
					l = strlen(inputStr);
					if (inputStr[0] == 'e')
						break;
					else if (inputStr[0] == 'a')
						break;
					else if (inputStr[0] == 'A')
						break;

					if (islower(inputStr[l - 1])) {
						sscanf(inputStr, "%d%c", &row, &ccol);
						col = ccol - 'a' + 1;
						returnScoreArray.push_back(bestScoreArray[size]);
						returnScoreArray[size].cellPtr = &board[row][col];
						size++;
					} else {
						sscanf(inputStr, "%d", &n);
						if (n < 0) {
							modifyDebugFeatures(n);
							break;
						} else {
							returnScoreArray.push_back(bestScoreArray[n]);
							debug.id.push_back(n);
							size++;
						}
					}
				}
			} while (n < 0);
			if (inputStr[0] == 'e') {
				debug.id.clear();
				for (unsigned int i = 0; i < returnScoreArray.size(); i++) {
					debug.id.push_back(i);
				}
				bestScoreArray.clear();
				bestScoreArray = returnScoreArray;
			} else {
				// a or A
				returnScoreArray.clear();
				for (int i = 0; i < thisNodeCnt; i++) {
					returnScoreArray.push_back(bestScoreArray[i]); // initialize with prelim
				}
				if (inputStr[0] == 'a')
					debug.id.clear();
			}
			thisNodeCnt = returnScoreArray.size();
		}
		for (int i = 0; i < (int) (returnScoreArray.size()); i++) {
			if (debug.find(i))
				cout << "Debug ";
			else
				cout << "      ";

			cout << returnScoreArray[i] << endl;
		}
		if (inputRemain == 0) {
			cout << "Is debug specification correct?" << endl;
			cin >> ach;
		} else {
			ach = 'y';
		}
	} while (ach != 'y');
	return thisNodeCnt;
}

scoreElement caro::evalAllCell(int currPlay, int in_NodeCnt, int in_depth,
		int min_depth, bool isMax, aScore alpha, aScore beta, bool debugThis,
		bool &redo, traceCell * callerTrace, tracer *headTracer) {
	caro backup(15);

	cell *cPtr;
	scoreElement bestScore, returnScore, termScore;
	vector<scoreElement> bestScoreArray, returnScoreArray;
	int nextPlay = oppositeVal(currPlay);
	int terminated = 0;
	int thisNodeCnt = in_NodeCnt;
	debugid debug;
	bool redoNext;
	int nextLevelWidth = in_NodeCnt; //nextWidth(depth,width);
	traceCell currTrace;
	currTrace.prev = callerTrace;
	currTrace.cell = nullptr;
	debugCell = false;
	aScore previousMovePoints;

	if (debugThis) {
		cout << "__________________" << endl;
		printTrace();
	}
	if (lowerMin) {
		in_NodeCnt -= 2;
		lowerMin = 0;
	}
	if (higherMin) {
		in_NodeCnt += 2;
		higherMin = 0;
	}
	termScore.myScore = termScore.oppScore = 0;
	if (0) {
		print(SYMBOLMODE2);
		print(SCOREMODE);

		char ach;
		cin >> ach;
	}
	// reset boardScore -- this is score for the previous move made
	previousMovePoints = {0,0,0};
	int highestConnected = 0;

	for (int row = 1; row < size; row++) {
		if (terminated && (in_depth >= min_depth))
			break;
		for (int col = 1; col < size; col++) {
			if (board[row][col].val & (E_NEAR | E_TNEAR)) {
				debugCell = inspectCell && ((row == iC[0]) && (col == iC[1]));
				bestScore = score1Cell(currPlay, row, col, debugThis);
				previousMovePoints += bestScore;
				previousMovePoints.connectedOrCost = in_depth;
				if (abs(bestScore.connectedOrCost) > 2) {
					if (abs(bestScore.connectedOrCost)
							> abs(highestConnected)) {
						highestConnected = bestScore.connectedOrCost;
					}
					if (debugThis) {
						cout << "HC=" << highestConnected << "-C"
								<< bestScore.connectedOrCost << endl;
					}
				}
				bestScore.cellPtr = &board[row][col];
				if (bestScore.bestMove() > 0)
					bestScoreArray.push_back(bestScore);
				// at EVAL --  only terminate if own play is winning

				bestScore.connectedOrCost = in_depth;
				if (bestScore.myScore >= MAGICNUMBER) {
					if (isMyPlay(currPlay)) { // 100% verified -- Do Not Change
						termScore = bestScore;
						previousMovePoints.oppScore =
								previousMovePoints.oppScore >> 1;
						terminated = 1;
						if (debugThis)
							cout << "AI-Win=" << bestScore;
						if (in_depth >= min_depth)
							break;
					}
				} else if (bestScore.oppScore >= MAGICNUMBER) {
					if (!(isMyPlay(currPlay))) {
						termScore = bestScore;
						previousMovePoints.myScore = previousMovePoints.myScore
								>> 1;
						terminated = 1;
						if (debugThis)
							cout << "Human-win=" << bestScore;
						if (in_depth >= min_depth)
							break;
					}
				}
			}
		}
	}

	if (0) {
		print(SYMBOLMODE2);
		print(SCOREMODE);
		char ach;
		cin >> ach;
	}
	if (in_depth < 0) {
		int i;

		sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
		for (i = 0; i < (int) bestScoreArray.size(); i++) {
			if (bestScoreArray[i].bestMove() <= MINSCORE)
				break;
			possMove[i] = bestScoreArray[i].cellPtr;
			if (i >= 39)
				break;
		}

		possMove[i] = nullptr;
		print(SCOREMODE);

		print(possMove);
		print(SYMBOLMODE);
		return bestScore;
	} else if (terminated) {
		bestScore = previousMovePoints;

		if (0 && (debugScoring || debugThis)) {
			printDebugInfo(termScore.cellPtr->rowVal, termScore.cellPtr->colVal,
					&currTrace, in_depth);
			printf("nextPlay=%c, aiPlay=%c ", convertToChar(currPlay),
					convertToChar(my_AI_Play));
			cout << " score=" << bestScore << " " << previousMovePoints
					<< "TERMINATED" << endl;
		}

	} else {
// SORT picking the best move. NOT bestValue

		int swidth = setWidthDepth[highestConnected][0];
		int sdepth = setWidthDepth[highestConnected][1];
		sdepth = moreDepth ? sdepth + 2 : sdepth;

		if (in_depth > (maxDepth - sdepth)) {
			min_depth = (maxDepth - sdepth);
			if (min_depth < maxSearchDepth)
				min_depth = maxSearchDepth;
		}
		in_NodeCnt = swidth;

		int passStart = (in_depth == maxDepth) ? 1 : 1;
		for (int pass = passStart; pass < 2; pass++) {

			thisNodeCnt = createNodeList(thisNodeCnt, in_NodeCnt,
					bestScoreArray, highestConnected, bestScore);

			if (debugThis || debugScoringd || debugScoringAll
					|| (in_depth == maxDepth)) {
				if (debugThis) {
					print(SYMBOLMODE2);
					print(SCOREMODE);
				}
				cout << "DebugSetup " << "Depth= " << in_depth;
				cout << headTracer << endl;
				if (debugThis) {
					cout << "w=" << swidth << ", d=" << sdepth
							<< ", in_depth = " << in_depth << ", min_depth="
							<< min_depth << endl;
				}
				thisNodeCnt = debugSetup(thisNodeCnt, in_depth, currPlay,
						(in_depth == maxDepth), debugThis, terminated,
						bestScoreArray, debug, returnScoreArray);
			}
// unless this is the last depth, playing the next hand (of the previous best fews)
// Recursively call to evaluate that play. The next call is for the opponent hand.
//	bestScore.connectedOrCost = depth;
			tracer * tptr =
					new tracer(
							&board[bestScore.cellPtr->rowVal][bestScore.cellPtr->colVal]);
			tptr->prev = headTracer;

			//	headTracer->next = tptr;
			bestScore = previousMovePoints;
//--------------------------------------
			if (docheck)
				save(backup);
			int adjustDepth = 0;
			auto topLevelStart = high_resolution_clock::now();
			bool tooLong = false;
			if ((terminated == 0) && (in_depth > min_depth)) {
				if (isMax)
					bestScore = dworstValue();
					else
					bestScore = dbestValue();
					vector <tracer *>tracerArrayPtr(thisNodeCnt);
					if(in_depth == maxDepth) {
						desiredRuntime = runtimeInMicroSecond/(thisNodeCnt)/8;
					}
					for (int i = 0; i < thisNodeCnt; i++) {
						if(terminated) {
							cout << "Terminated" << endl;
							break;
						}
						cPtr = bestScoreArray[i].cellPtr;
						tracerArrayPtr[i] = new tracer(&board[cPtr->rowVal][ cPtr->colVal]);
						if(i==0) {
							headTracer->next = tracerArrayPtr[i];
						}
						tracerArrayPtr[i]->prev = headTracer;
						tracerArrayPtr[i]->atDepth = in_depth-1;
						currTrace.cell = & board[cPtr->rowVal][ cPtr->colVal];
						aScore saveScores[8][saveRestoreDist],s1Score;
						int saveVals[8][saveRestoreDist],s1Val;
						saveScoreVal( cPtr->rowVal, cPtr->colVal,
								saveScores, s1Score,
								saveVals,s1Val);
						redoNext = false;
						setCell(currPlay, cPtr->rowVal, cPtr->colVal, E_TNEAR);
						do {
							bool debugNext = false;
							debugNext = debug.find(i);
							if(in_depth == maxDepth) {
								start = high_resolution_clock::now();
							}
							int new_depth = min_depth+adjustDepth;

							returnScore = evalAllCell(nextPlay, nextLevelWidth, in_depth - 1,
									new_depth,
									!isMax, alpha, beta, debugNext, redoNext,
									&currTrace, tracerArrayPtr[i]);

							if (redoNext)
							delete tracerArrayPtr[i]->next;
						}while(redoNext);
						bestScoreArray[i] = (aScore)returnScore; // only taking the score, not cellptr
						tracerArrayPtr[i]->savePoint = returnScore;
						if (debugThis) {
							if(isMax)
							cout << "\nMAX-";
							else
							cout << "\nMIN-";
							printf("i=%d,",i);
							returnScoreArray[i] = returnScore;
						}
						bool found_better =
						isMax ?
						returnScore.greaterValueThan(bestScore):
						bestScore.greaterValueThan(returnScore);
						if (found_better) {
							headTracer->next = tracerArrayPtr[i];
							bestScore = returnScore;
							bestScore.cellPtr = bestScoreArray[i].cellPtr;
							//	cout << "bestScore=" << bestScore;

						} else

						if(isMax) {
							alpha = asMAX(alpha, bestScore);
						} else {
							beta = asMIN(beta, bestScore);
						}
						auto stop = high_resolution_clock::now();
						auto duration = duration_cast<microseconds>(stop - start);
						if(debugThis || (in_depth == maxDepth) ) {
							cout << endl << *bestScoreArray[i].cellPtr << returnScore << "BS=" << bestScore;
						}
						if((debugThis==0) && !underDebug) {
							if(in_depth == maxDepth) {
								int ratio = (duration.count()*50 / desiredRuntime);
								int save_adjustDepth = adjustDepth;
								auto tduration = duration_cast<microseconds>(stop - topLevelStart);
								tooLong = tduration.count() > runtimeInMicroSecond;
								if(tooLong) {
									cout << " Total exe time too long, i =" << i << " out of " << thisNodeCnt << endl;
									thisNodeCnt = i+1;
								}
								if(ratio <=1) { // quick execution time
									adjustDepth -=2;// increase run time by reducing min_depth, take care not to increase too fast
								} else if(ratio > 200) { // Long
									adjustDepth +=2;// reduce runtime
								}
								if(abs(adjustDepth) > 10) {
									adjustDepth = save_adjustDepth;
								}
								if((adjustDepth+min_depth) > in_depth)
								adjustDepth = in_depth - min_depth -2;
							} else {
								//tooLong = duration.count() > desiredRuntime*16;
								tooLong = 0;
								if(tooLong) {
									if(in_depth == maxDepth-1)
									cout << "\nleaf exe time too long="<<duration.count() << " at depth ="
									<< in_depth << " i =" << i << " out of " << thisNodeCnt;
								}
								adjustDepth = 0;
							}
						} else {
							adjustDepth = 0;
						}
						restoreScoreVal( cPtr->rowVal, cPtr->colVal,
								saveScores, s1Score,
								saveVals,s1Val);
						if ((alpha.greaterValueThan(beta))) {
							terminated = 1;
							thisNodeCnt = i+1;
						}
						if(docheck && (oneShot && !(compare(backup)))) {
							char ach;
							cout << " At depth=" << in_depth << " W="<< i << endl;
							printDebugInfo(cPtr->rowVal, cPtr->colVal,&currTrace, in_depth);
							cout << *tracerArrayPtr[i];

							cin.clear();
							cin.ignore(numeric_limits<streamsize>::max(), '\n');
							cin >> ach;
							terminated = 1;
							thisNodeCnt = i+1;
							oneShot = false;

						}
						if(tooLong && (debugThis==0) && !underDebug) {
							cout << "brk-"<<in_depth;
							break;
						}
					}

					if (debugThis||(in_depth == maxDepth)) {
						printTrace();
						if (isMax)
						cout << "Max";
						else
						cout << "Mini";
						cout << endl;
						char redoCh =' ';
						cout << "NodeCnt=" << thisNodeCnt << endl;

						do {
							for (int i = 0; i < thisNodeCnt; i++) {
								cout <<"______________________________________________________________________________\n";
								printf("\t<<%d,%d>>", in_depth, i);
								printf("%C| ", convertCellToStr(currPlay) );
								cout << bestScoreArray[i];
								cout <<"I-"<<i<< " " << *tracerArrayPtr[i] << endl;

								hist histArray;
								tracerArrayPtr[i]->extractTohistArray(currPlay,bestScoreArray[i].cellPtr, histArray);
								print(histArray);
							}
							cout << " bestScore=" << bestScore << endl;
							hist histArray;
							// print trace from previous level, hence nextPlay instead of currPlay
							headTracer->next->extractTohistArray(currPlay,bestScore.cellPtr, histArray);
							print(histArray);
							cout << endl;
							if(debugThis) {
								cout << "Redo?" << endl;
								cin >> redoCh;
							} else {
								redoCh = 'n';
							}
							redo = ((redoCh == 'Y') || (redoCh == 'y'));

						}while(redoCh == 'i');
					}
					//only retain the 'keep' path, code to delete the discard paths
					for (int j = 0; j < thisNodeCnt; j++) {
						if(headTracer->next != tracerArrayPtr[j]) {
							if(tracerArrayPtr[j]) {
								delete tracerArrayPtr[j];
							}
						}
					}

				}
			}
		}

	if ((in_depth > (min_depth)) && 0) {
		cout << "depth=" << in_depth << " max=" << isMax << "bestScore ="
				<< bestScore << endl;
		char ach;
		cin >> ach;
	}

	return bestScore;
}

hashTable::hashTable() {
	arrayE_cnt = 0;
}
void swap2E(hEntry &a, hEntry &b) {
	hEntry t;
	t = a;
	a = b;
	b = t;
}
void hashTable::addEntry(int line, int connected, int bitcnt, int score) {
	int i;
	lowest = 99999999;
	lowestI = 99999999;

	for (i = 0; i < arrayE_cnt; i++) {
		if (arrayE[i].refcnt < lowest) {
			lowest = arrayE[i].refcnt;
			lowestI = i;
		}
		if ((arrayE[i].line == line) && (arrayE[i].bitcnt == bitcnt)) {
			if ((arrayE[i].refcnt++ > lowest) && (i > lowestI)) {
				swapcnt++;
				swap2E(arrayE[lowestI], arrayE[i]);
			}
			return;
		}
	}
	if (i > 1280) {
		cout << "hashE cnt too small" << endl;
	} else {
		arrayE[arrayE_cnt].line = line;
		arrayE[arrayE_cnt].connected = connected;
		arrayE[arrayE_cnt].bitcnt = bitcnt;
		arrayE[arrayE_cnt].refcnt = 1;
		arrayE[arrayE_cnt++].score = score;
		if (debugHash) {
			int i = arrayE_cnt - 1;
			char binary[9];
			int val = arrayE[i].line;
			toBinary(val, binary);
			printf("HASH %d %8x %8d %8d %8d %8d %8s\n", i, arrayE[i].line,
					arrayE[i].connected, arrayE[i].bitcnt, arrayE[i].score,
					arrayE[i].refcnt, binary);
		}

	}
}

void hashTable::print() {
	cout << "Hash Table: count = " << arrayE_cnt << "swapcnt =" << swapcnt
			<< endl;
	swapcnt = 0;
	printf("%4s %8s %8s %8s %8s %8s %8s", "no", "Line", "connected", "bitcnt",
			"score", "RefCnt", "Binary");
	cout << endl;
	for (int i = 0; i < arrayE_cnt; i++) {
		char binary[9];
		int val = arrayE[i].line;
		toBinary(val, binary);
		printf("%4d %8x %8d %8d %8d %8d %8s\n", i, arrayE[i].line,
				arrayE[i].connected, arrayE[i].bitcnt, arrayE[i].score,
				arrayE[i].refcnt, binary);
	}
}
