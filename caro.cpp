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

using namespace std;

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
#define MAGICNUMBER NUM6*16
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
#define percentAdjust(val,percent) (val*percent)/128
// (a+b)/2 =50 percent -> a+b = 2*p
#define fscale(a,b,p)  a+((b-a)*p)/100

#include "caro.h"
int search_depth = 5;
int search_width = 5;

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

#define dbestValue() {99999999,-99999999}
#define dworstValue() {-9999999,+99999999}
#define dbestMove() {99999999,99999999}
#define dworstMove() {-99999999,-99999999}
#define isMyPlay(setVal) setVal==myVal
tsDebug aDebug, bestScoreDebug;
int gdebug = 0;
int debugScoring = 0;
int debugScoringd = 0;
int debugScoringAll = 0;
int debugAllPaths = 0;

int debugBestPath = 0;
int debugHash = 0;
int debugAI = 0;
int debugAIbest = 0;
int debugRow, debugCol;
int interactiveDebug;
int debugTrace = 0;

tScore::tScore() {
	val = 0;
	defVal = 0;
	ts_ret = {0,0};
	cellPtr = nullptr;
}

tScore::tScore(scoreElement a) {
	val = a.val;
	defVal = a.defVal;
	cellPtr = a.cellPtr;
	ts_ret = {0,0};
}

hashTable ahash;
tsDebug::tsDebug() {
	for (int i = 0; i < 40; i++) {
		for (int j = 0; j < 40; j++) {
			Array[i][j].val = Array[i][j].defVal = 0;
			Array[i][j].ts_ret = {0,0};
			Array[i][j].cellPtr = nullptr;
		}
	}
}

void caro::reset() {
	moveCnt = 0;
	last2p = 0;
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
			board[row][col].score.val = 0;
			board[row][col].score.defVal = 0;
		}
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */

void caro::undo1move() {
	cout << "Undo p=" << last2p;
	if ((last2p - 2 != saveLast2p) && moveCnt > 0) {
		moveCnt -= 2;
		last2p = (last2p - 2) & 0xff;
		printf("\tv=%x, r=%d, c=%d\n", last2v[last2p + 2],
				last2[last2p + 2]->rowVal, last2[last2p + 2]->colVal);
		restoreCell(last2v[last2p + 2], last2[last2p + 2]->rowVal,
				last2[last2p + 2]->colVal);
		printf("\tv=%x, r=%d, c=%d\n", last2v[last2p + 1],

		last2[last2p + 1]->rowVal, last2[last2p + 1]->colVal);

		restoreCell(last2v[last2p + 1], last2[last2p + 1]->rowVal,
				last2[last2p + 1]->colVal);
	}
}
void caro::redo1move() {
	cout << "redo p=" << last2p;
	if (last2p + 2 != saveLast2p) {
		moveCnt += 2;
		last2p = (last2p + 2) & 0xff;
		printf("\tv=%x, r=%d, c=%d\n", last2v[last2p + 2],
				last2[last2p + 2]->rowVal, last2[last2p + 2]->colVal);
		restoreCell(last2v[last2p + 2], last2[last2p + 2]->rowVal,
				last2[last2p + 2]->colVal);
		printf("\tv=%x, r=%d, c=%d\n", last2v[last2p + 1],
				last2[last2p + 1]->rowVal, last2[last2p + 1]->colVal);

		restoreCell(last2v[last2p + 1], last2[last2p + 1]->rowVal,
				last2[last2p + 1]->colVal);
	}
}
/*
 * extracting a line for the purpose of scoring
 * This will need to work with how scoring is done.  Extracting will need to go
 * in hand.  So, this will need to change depend on the method
 * ONGOING CODING -- NOT DONE
 */
Line caro::extractLine(int inVal, int dir, int row, int col, bool &ending,
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
			&& (debugScoringAll || cdbg.ifMatch(&board[row][col], -1, 1));
	oppval = oppositeVal(board[row][col].val);
	//debugLine = true;
	cell *currCell = &board[row][col];
	cell *oriCell = currCell;
	cell *backoffCell;	//, *lastone;
	//currCell = currCell->near_ptr[ReverseDirection(dir)];

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool prevVal = false;
	aline.type = val;
	int freeEnd = 0;
	if (debugLine) {
		cout << "Dir=" << dir << " myVal=" << myVal << " val=" << val;
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
	int marked = 0;
	int continuous = 0;
	ending = false;
	for (int i = 0; i < (SEARCH_DISTANCE); i++) {
		if (debugLine)
			cout << *currCell << "-";
		aline.val = aline.val << 1;
		aline.cnt++;
		prevVal = currCell->val == val;
		if (currCell->val == val) {
			continuous++;
			if (continuous >= 5)
				ending = true;
			bitcnt++;
			aline.val = aline.val | 0x1;
			if (unconnected == 1) {
				aline.connected = save + 1;
				if (aline.connected < 3)
					marked++;
			} else
				aline.connected++;
			unconnected = 0;
		} else {
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

	aline.cnt += freeEnd * 2;

	if (save > aline.connected)
		aline.connected = save;
	/*
	 if ((aline.connected > 2) && (marked >= 1)) {
	 aline.connected -= marked - 1;
	 }
	 */
	if (aline.connected == 1)
		aline.connected = 0;
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt < 5)
		aline.connected = 0;
	if (aline.connected < 4) {
		aline.offset -= marked;
	}
	aline.connected = (aline.connected * 2);

	if (aline.blocked) {
		if (aline.connected < 8)
			aline.offset--;
	}
	if ((val == myVal) && (aline.connected >= 6))  // Favor offensive
		aline.offset++;

	/*
	 if ((val == myVal) && (aline.connected > 0))
	 aline.connected--; // go first, assume that opponent to block  (1 less)
	 */
	if (debugLine) {
		cout << endl;
	}
	return aline;
}

int Line::evaluate(bool ending) {
// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
// self learning)
	score = 0;
	int bitcnt = 0;
	if (cnt < 5)
		score = 0;
	else if (ending) {
		score = MAGICNUMBER * 2;
	} else {
		int tval = val;
		while (tval) {
			bitcnt = bitcnt + (tval & 1); // recalculate bitcnt
			tval = tval >> 1;
		}
		score = bitcntArrayscale[connected + offset];
//	score = percentAdjust(score, adjustCntArray[bitcnt]);
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

aScore caro::score1Cell(int inVal, int row, int col, int depth,
		bool debugThis) {
	FourLines astar;
	aScore rtn;

	bool saveCheck = false;
	bool redothisone = false;
	aScore testSaveScore = { 0, 0 };
	if (board[row][col].val & E_CAL) {
		skipcnt++;

		return board[row][col].score;

		saveCheck = true;
		cout << "saved ";
		testSaveScore = board[row][col].score;
	}
	scorecnt++;
	do {
		int scores[2] = { 0, 0 };
		int opnVal = oppositeVal(myVal);
		int tempVal[2];
		bool ending;
		tempVal[0] = myVal;
		tempVal[1] = opnVal;

		//	saveScoreVal(row, col, saveScores, saveVals);
		int saveVal = board[row][col].val;

		for (int j = 0; j < 2; j++) {
			int setVal = tempVal[j];
			int ill_6, ill_4, ill_3;
			ill_6 = ill_4 = ill_3 = 0;
			setCell(setVal, row, col, E_FAR);
			for (int dir = East; dir < West; dir++) {
				evalCnt++;
				astar.Xlines[dir] = extractLine(inVal, dir, row, col, ending,
						debugThis);
				int tscore = astar.Xlines[dir].evaluate(ending);
				//	if (aDebug.enablePrinting||cdbg.ifMatch(&board[row][col],depth,0)) {

				if (debugThis)
					cout << "-" << tscore;
				/*
				 * to score cross-coupling
				 */
				if ((tscore > NUM1) && (scores[j] > NUM1))
					if (tscore < scores[j])
						scores[j] = scores[j] * 3 + tscore * 2; //  is too much?
					else
						scores[j] = scores[j] * 2 + tscore * 3; //  is too much?

				else
					scores[j] = scores[j] + tscore;
				if (debugThis)
					cout << "=" << scores[j];
				if (astar.Xlines[dir].connected >= (6 * 2))
					ill_6 = 1;
				if (astar.Xlines[dir].connected == (3 * 2))
					ill_3++;
				if (astar.Xlines[dir].connected == (4 * 2))
					ill_4++;
			}

			if ((ill_6 || (ill_4 > 1) || (ill_3 > 1)) && setVal == X_) {
				scores[j] = 0; //-0x200; TODO
			}

			if (setVal == myVal)
				myMoveAccScore += scores[j];
			else
				opnMoveAccScore += scores[j];
			// doing this here intead of after for loop bc of how setcell works
			board[row][col].val = saveVal;
		}

		rtn.val = scores[0];
		rtn.defVal = scores[1];
		board[row][col].score = rtn;
		board[row][col].val |= E_CAL; // will not be re-evaluate
		/* TODO: still have problem with E_CAL, too few saving
		 *
		 */

		if (saveCheck) {
			if (redothisone) {
				debugScoringAll = 0;
				redothisone = false;
				break;
			}
			if (!(testSaveScore == rtn)) {

				cout << "SHORTCUT different, saveScore =" << testSaveScore
						<< " newScore =" << rtn;
				printDebugInfo(row, col, nullptr);
				print(SYMBOLMODE2);
				redothisone = true;
				debugScoringAll = 1;
			}
		} else {
			/*
			 cout << board[row][col] << rtn ;
			 cout << "LastCell =" << *lastCell << "T-" << hex << lastCellType << " " <<endl;
			 */
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
	return ((a.val + a.defVal) > (b.val + b.defVal));
}
bool betterValue(scoreElement & a, scoreElement & b) {
	return ((a.val - a.defVal) > (b.val - b.defVal));
}
bool lessValue(scoreElement & a, scoreElement & b) {
	return ((a.val - a.defVal) < (b.val - b.defVal));
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

	case -3:
		cout << "Turn " << FLIP(debugScoring) << " debugScoring" << endl;

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
scoreElement caro::evalAllCell(int setVal, int width, int depth,
		int currentWidth, bool isMax, int alpha, int beta, breadCrumb &parent,
		bool debugThis, bool &redo, traceCell * callerTrace) {

	cell *cPtr;
	scoreElement bestScore, returnScore, termScore;
	vector<scoreElement> bestScoreArray, returnScoreArray;
	int opnVal = oppositeVal(setVal);
	int foundPath = -8888;
	int terminated = 0;
	int debugId = 99999;
	bool redoNext;
	int nextLevelWidth = nextWidth(depth,width);
	traceCell currTrace;
	currTrace.prev = callerTrace;
	currTrace.cell = nullptr;
	if (debugThis) {
		cout << "__________________" << endl;
		printTrace();
	}
	termScore.val = termScore.defVal = 0;
	if (0) {
		print(SYMBOLMODE2);
		print(SCOREMODE);

		char ach;
		cin >> ach;
	}
	for (int row = 1; row < size; row++) {
		if (terminated)
			break;
		for (int col = 1; col < size; col++) {
			if (board[row][col].val & (E_NEAR | E_TNEAR)) {
				bestScore = score1Cell(setVal, row, col, depth, debugThis);
				bestScore.cellPtr = &board[row][col];
				bestScoreArray.push_back(bestScore);
				// at EVAL --  only terminate if own play is winning
				if (debugThis)
					cout << bestScore << endl;
				// TODO still dont know how to terminzte
				if (bestScore.val >= MAGICNUMBER) {
					if (isMyPlay(setVal)) {
						termScore = bestScore;
						terminated = 1;
						break;
					}
				} else if (bestScore.defVal >= MAGICNUMBER) {
					if (isMyPlay(opnVal)) {
						termScore = bestScore;
						terminated = 1;
						break;
					}
				}
			}
		}
	}// TODO greedy algo does not work
	if (0) {
		print(SYMBOLMODE2);
		print(SCOREMODE);
		char ach;
		cin >> ach;
	}

	if (terminated) {
		if (debugScoring || debugThis) {
			printDebugInfo(termScore.cellPtr->rowVal, termScore.cellPtr->colVal,
					&currTrace);
			printf("setVal=%d, myVal=%d ", setVal, myVal);
			cout << " score=" << bestScore << "TERMINATED" << endl;
		}
	}

	if (terminated) {
		bestScore = termScore;
		parent.top.ptr = bestScore.cellPtr;
		parent.top.width_id = 0;
		parent.top.depth_id = depth;
		parent.top.val = setVal;
		return (bestScore);
	} else {
// SORT picking the best move. NOT bestValue
		sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
		bestScore = bestScoreArray[0]; // BestScore
		parent.top.ptr = bestScoreArray[0].cellPtr;
	}
	parent.top.width_id = 0;
	parent.top.depth_id = depth;
	parent.top.val = setVal;

	widthAtDepth[depth] = MIN(width, (int )bestScoreArray.size()); // size of bestScoreArray can be smaller than "width"
	if (debugThis || debugScoringd || debugScoringAll || debugAllPaths
			|| (currentWidth == aDebug.debugWidthAtDepth[depth])) {
		print(SCOREMODE);
		for (int i = 0; i < widthAtDepth[depth]; i++) {
			//		returnScoreArray.push_back(bestScoreArray[i]); // initialize with prelim score

			if (depth < aDebug.lowDepth)
				aDebug.lowDepth = depth;
			aDebug.Array[depth][i] = bestScoreArray[i];
			aDebug.Array[depth][i].cellPtr = bestScoreArray[i].cellPtr;
			cPtr = bestScoreArray[i].cellPtr;
			if (debugThis || debugScoring || debugBestPath || debugScoringAll
					|| debugAllPaths) {
				printf("\t<%d,%d>", depth, i);
				printf("%C", convertCellToStr(setVal));
				cout << bestScoreArray[i] << "|";
				if (i % 2 == 0)
					printf("\n");
			}
		}
		if (debugThis && (terminated == 0)) {
			int n;
			int l, row, col;
			char numstr[20], ccol;

			debugId = 0;
			do {
				n = 1;

				printDebugInfo(0, 0, &currTrace);
				cout << endl << "Enter # for debug" << endl;

				for (int size = 0; size < widthAtDepth[depth];) {
					cin >> numstr;
					l = strlen(numstr);
					if (numstr[0] == 'e')
						break;
					if (numstr[0] == 'a') {
						debugId = 100;
						break;
					}
					if (islower(numstr[l - 1])) {
						sscanf(numstr, "%d%c", &row, &ccol);
						col = ccol - 'a' + 1;
						returnScoreArray.push_back(bestScoreArray[size]);
						returnScoreArray[size].cellPtr = &board[row][col];
						size++;
					} else {
						sscanf(numstr, "%d", &n);
						if (n < 0) {
							modifyDebugFeatures(n);
							break;
						} else {
							returnScoreArray.push_back(bestScoreArray[n]);
							size++;
						}
					}

				}

			} while (n < 0);
			if (numstr[0] == 'e') {
				bestScoreArray.clear();
				bestScoreArray = returnScoreArray;
				widthAtDepth[depth] = returnScoreArray.size();
			} else {
				for (int i = 0; i < widthAtDepth[depth]; i++) {
					returnScoreArray.push_back(bestScoreArray[i]); // initialize with prelim
				}
			}
		}
	}
// debug stuff

// unless this is the last depth, playing the next hand (of the previous best fews)
// Recursively call to evaluate that play. The next call is for the opponent hand.

	if ((terminated == 0) && (depth > 0)) {
		if (isMax)
			bestScore = dworstValue();
			else
			bestScore = dbestValue();
			breadCrumb myCrumb(depth - 1);

			for (int i = widthAtDepth[depth]-1; i>=0; i--) {
				prevD = depth;
				cPtr = bestScoreArray[i].cellPtr;
				currTrace.cell = & board[cPtr->rowVal][ cPtr->colVal];

				aScore saveScores[8][saveRestoreDist],s1Score;
				int saveVals[8][saveRestoreDist],s1Val;

				saveScoreVal( cPtr->rowVal, cPtr->colVal,
						saveScores, s1Score,
						saveVals,s1Val);

				/////// trace for debug -- DONOT REMOVE
				trace.push_back((unsigned char)i);
				cdebug.tracePush(i);
				redoNext = false;
				if(cdebug.traceMatch(cPtr->rowVal, cPtr->colVal)) {

					print(SYMBOLMODE2);
					cout << "---------------------------TRACE MATCH---------------" << endl;
				}
				setCell(setVal, cPtr->rowVal, cPtr->colVal, E_TNEAR);
				if(cdebug.traceMatch(cPtr->rowVal, cPtr->colVal)) {
					print(SYMBOLMODE2);
					cout << "---------------------------TRACE MATCH---------------" << endl;
				}
				do {
					bool debugNext = (debugId == i);
					returnScore = evalAllCell(opnVal, nextLevelWidth, depth - 1, i + foundPath,
							!isMax, alpha, beta, myCrumb,debugNext, redoNext,
							&currTrace);
				}while(redoNext);
				/////// trace for debug -- DONOT REMOVE
				cdebug.tracePop();
				trace.pop_back();
				if (debugThis) returnScoreArray[i] = returnScore;

				bool found_better =
				isMax ?
				betterValue(returnScore,bestScore):
				betterValue(bestScore,returnScore);

				if (found_better) {
					if(debugThis||debugScoring) {
						printf("i=%d,",i);
						cout << "bestScore=" << bestScore;
						cout << " Return=" << returnScore << endl;

					}
					bestScore = returnScore;
					bestScore.cellPtr = bestScoreArray[i].cellPtr;
					parent = myCrumb;
					parent.top.ptr = bestScoreArray[i].cellPtr;
					parent.top.width_id = i;
				} else
				if(debugThis||debugScoring)
				cout << "\t\t Return=" << returnScore << endl;

				if(isMax) {
					alpha = MAX(alpha, bestScore.bValue());} else {
					beta = MIN(beta, bestScore.bValue());}
				//	restoreCell(saveVal, cPtr->rowVal, cPtr->colVal);
				if(debugScoring||debugThis) {
					if ((beta<=alpha) || (bestScore.getScore(opnVal,myVal) >= 100*MAGICNUMBER)) {
						// quit if setVal's score is MAGIC
						printDebugInfo(cPtr->rowVal, cPtr->colVal,&currTrace);
						if(beta<=alpha) {
							cout << "ALPHA=" << alpha << " BETA=" << beta << " TERMINATE" << endl;
						} else {
							cout <<bestScore << " Terminated" << endl;
							print(SCOREMODE);
						}
					}
				}

				restoreScoreVal( cPtr->rowVal, cPtr->colVal,
						saveScores, s1Score,
						saveVals,s1Val);
				if ((beta<=alpha) || (bestScore.getScore(opnVal,myVal) >= 100*MAGICNUMBER)) {
					terminated = 1;
					break;
				}
			}
			if (debugThis) {
				printTrace();

				if (isMax)
				cout << "Max ";
				else
				cout << "Mini";

				for (int i = 0; i < widthAtDepth[depth]; i++) {
					cPtr = aDebug.Array[depth][i].cellPtr;
					if (debugThis||debugScoring || debugBestPath || debugScoringAll
							|| debugAllPaths) {
						printf("\t<<%d,%d>>", depth, i);
						printf("%C| ", convertCellToStr(
										setVal)
						);
						cout << bestScoreArray[i] << "---";
						cout << returnScoreArray[i] << endl;
					}
				}

				cout << " bestScore=" << bestScore;
				cout << endl;
				cout << "Redo?" << endl;
				char redoCh;
				cin >> redoCh;
				redo = ((redoCh=='Y')||(redoCh=='y'));

				print(SYMBOLMODE2);
				print(SCOREMODE);

			}

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
	for (int i; i < arrayE_cnt; i++) {
		char binary[9];
		int val = arrayE[i].line;
		toBinary(val, binary);
		printf("%4d %8x %8d %8d %8d %8d %8s\n", i, arrayE[i].line,
				arrayE[i].connected, arrayE[i].bitcnt, arrayE[i].score,
				arrayE[i].refcnt, binary);
	}
}
