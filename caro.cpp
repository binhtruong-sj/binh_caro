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
#define MAGICNUMBER NUM6*128
#define NUM6 NUM5*64
#define NUM5 NUM4*32
#define NUM4 NUM3*16
#define NUM3 NUM2*8
#define NUM2 0x6
#define NUM1 0

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
int bitcntArrayscale[] = { 0, fscale(0, NUM1, 50), NUM1, fscale(NUM1, NUM2, 90),
NUM2, fscale(NUM2, NUM3, 90), NUM3, fscale(NUM3, NUM4, 90), NUM4, fscale(NUM4,
		NUM5, 90),
NUM5, fscale(NUM5, NUM6, 50), NUM6, NUM6, NUM6, NUM6, NUM6,
NUM6, NUM6 };

#define BIAS_PERCENT 100  // more than opponent
#define biasDefendAdjust(val) val*BIAS_PERCENT/100

#define dbestValue() {99999999,-99999999}
#define dworstValue() {-9999999,+99999999}
#define dbestMove() {99999999,99999999}
#define dworstMove() {-99999999,-99999999}

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

cell * last2[256]; // fixed at 256, change to use remeainder if different setting
int last2v[256];
int last2p = 0;
int saveLast2p;
int moveCnt = 0;

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
 * set a cell to X or O
 */
cell * caro::setCell(int setVal, int row, int col, int near) {
	if ((board[row][col].val & 0x3) == 0) {
		if (setCellCnt++ == printInterval) {
			setCellCnt = 0;
		}
		if (near == E_NEAR) { // Only real set can be UNDO
			evalCnt = 0;
			myMoveAccScore = 0;
			opnMoveAccScore = 0;
			moveCnt++;
			last2p = (last2p + 1) & 0xFF;
			saveLast2p = last2p;
			last2[last2p] = &board[row][col];
			last2v[last2p] = board[row][col].val;
		}
		board[row][col].val = setVal;
		board[row][col].score = {0,0};
		setNEAR(row, col, near);
		return &board[row][col];
	} else
		return nullptr;
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */
// TODO problem with recalculate function, not clear that bit
/*
 * Temporary near marking set when traverse further ahead
 */
void caro::setNEAR(int row, int col, int near) {
	cell *currCell;
	for (int dir = East; dir <= SEast; dir++) {
		currCell = &board[row][col];
		for (int i = 0; i < InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			currCell->val = currCell->val & ~(E_CAL);

			if ((currCell->val & 0x3) == 0x3)
				break;
			else if ((currCell->val & (O_ | X_ | E_NEAR)) == 0) {
				currCell->val = near;
			}
		}
	}
}
/*
 * After temporary setting a Cell to X' or O' for scoring, now return it and its neighbor to prev values
 */
cell * caro::restoreCell(int saveVal, int row, int col) {
	cell *currCell;
	board[row][col].val = saveVal; // Return the val to prev

	for (int dir = East; dir <= SEast; dir++) {
		currCell = &board[row][col];
		for (int i = 0; i < InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			if (currCell->val & E_TNEAR) {
				currCell->val = currCell->val & ~(E_CAL);
				currCell->score= {0,0};
				currCell->val = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
			}
		}
	}
	return currCell;
}

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
Line caro::extractLine(int inVal, int dir, int row, int col) {
	Line aline;
	int bitcnt = 0;
	aline.val = 0;
	aline.cnt = 0;
	int unconnected = 0;

	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	val = board[row][col].val;
	bool debugLine = debugScoringAll || cdbg.ifMatch(&board[row][col], -1, 1);
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
	int save = 0;
	if (debugLine) {
		cout << endl << board[row][col] << " --> ";
	}
	int marked = 0;
	for (int i = 0; i < (SEARCH_DISTANCE); i++) {

		aline.val = aline.val << 1;
		aline.cnt++;
		prevVal = currCell->val == val;
		if (currCell->val == val) {
			bitcnt++;
			aline.val = aline.val | 0x1;
			if (unconnected == 1) {
				aline.connected = save + 1;
				marked++;
			} else
				aline.connected++;
			unconnected = 0;
		} else {
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
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt >= 5) {
		if (bitcnt <= 4)
			aline.connected -= aline.blocked;
	} else
		aline.connected = 0;
	aline.connected = (aline.connected * 2) - marked;
	/*
	 if ((val == inVal) && (aline.connected >= 5))  // Favor offensive
	 aline.connected++;
	 */

	if ((val == myVal) && (aline.connected > 0))
		aline.connected--; // go first, assume that opponent to block  (1 less)

	if (debugLine) {
		cout << endl;
	}
	return aline;
}

int Line::evaluate() {
	// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
	// self learning)
	score = 0;
	int bitcnt = 0;
	if (cnt < 5)
		score = 0;
	else if (((connected >= 5 * 2) && (type == O_))
			|| ((connected == 5 * 2) && (type == X_))) {
		score = MAGICNUMBER * 2;
	} else {
		int tval = val;
		while (tval) {
			bitcnt = bitcnt + (tval & 1); // recalculate bitcnt
			tval = tval >> 1;
		}
		score = bitcntArrayscale[connected];
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

aScore caro::score1Cell(int inVal, int row, int col, int depth) {
	FourLines astar;
	aScore saveScores[8][InspectDistance], s1Score;
	int saveVals[8][InspectDistance], s1Val;

	if (board[row][col].val & E_CAL) {
		//	cout << "skip score1cell, score =" << board[row][col].score;
		skipcnt++;
		//	return board[row][col].score;
	}
	scorecnt++;
	int scores[2] = { 0, 0 };
	int opnVal = oppositeVal(myVal);
	int tempVal[2];
	tempVal[0] = myVal;
	tempVal[1] = opnVal;

	saveScoreVal(row, col, saveScores, s1Score, saveVals, s1Val);
	for (int j = 0; j < 2; j++) {
		int setVal = tempVal[j];
		int saveVal = board[row][col].val;
		int ill_6, ill_4, ill_3;
		ill_6 = ill_4 = ill_3 = 0;
		setCell(setVal, row, col, E_FAR);
		for (int dir = East; dir < West; dir++) {
			evalCnt++;
			astar.Xlines[dir] = extractLine(inVal, dir, row, col);
			int tscore = astar.Xlines[dir].evaluate();
			//	if (aDebug.enablePrinting||cdbg.ifMatch(&board[row][col],depth,0)) {
			if (aDebug.enablePrinting || debugScoringAll || debugScoringd) {
				cout << board[row][col] << "dir=" << dir;
				cout << astar.Xlines[dir] << endl;
				if (aDebug.printCnt++ % 40 == 0)
					prompt("Hit Enter Key");
			}
			//	cout << tscore << "=" << scores[j] << "+" << dir << "+ ";
			scores[j] = scores[j] + tscore;
			if (astar.Xlines[dir].connected >= 6 * 2)
				ill_6 = 1;
			if (astar.Xlines[dir].connected == 3 * 2)
				ill_3++;
			if (astar.Xlines[dir].connected == 4 * 2)
				ill_4++;
		}
		if ((ill_6 || (ill_4 > 2) || (ill_3 > 2)) && setVal == X_)
			scores[j] = 0; //-0x200; TODO

		if (setVal == myVal)
			myMoveAccScore += scores[j];
		else
			opnMoveAccScore += scores[j];

		restoreCell(saveVal, row, col);
	}
	// opponent is playing second, hence lower score
//	restoreScoreVal(row, col, saveScores, s1Score, saveVals, s1Val);

	aScore rtn;
	rtn.val = scores[0];
	rtn.defVal = scores[1];
	board[row][col].score = rtn;
	board[row][col].val |= E_CAL; // will not be re-evaluate

	return rtn;
}
bool betterMove(scoreElement & a, scoreElement & b) {
	return ((a.val + a.defVal) > (b.val + b.defVal));
}
bool betterValue(scoreElement & a, scoreElement & b) {
	return ((a.val - a.defVal) > (b.val - b.defVal));
}
/*
 *
 *
 */
scoreElement caro::evalAllCell(int setVal, int width, int depth,
		int currentWidth, bool maximizingPlayer, breadCrumb &parent,
		bool debugThis, bool &redo) {

	cell *cPtr;
	scoreElement bestScore, returnScore;
	vector<scoreElement> bestScoreArray, returnScoreArray;
	int opnVal = oppositeVal(setVal);
	int foundPath = -8888;
	int terminated = 0;
	int debugId = 99999;
	bool redoNext;
	for (int row = 1; row < size; row++) {
		for (int col = 1; col < size; col++) {
			if (terminated)
				break;
			if (board[row][col].val & (E_NEAR | E_TNEAR)) {
				bestScore = score1Cell(setVal, row, col, depth);
				{
					cout << board[row][col];
					print(SYMBOLMODE);
					print(SCOREMODE);

					char ach;
					cin >> ach;
				}
				if (debugScoring) {
					cout << board[row][col] << "score=" << bestScore << endl;
				}
				if (bestScore.getScore(setVal, myVal) >= MAGICNUMBER) {
					/*
					 cout << "TTT" << hex << bestScore.bestMove() << " " << hex
					 << MAGICNUMBER;
					 */
					terminated = 1; // not sure about this.
				}
				bestScore.cellPtr = &board[row][col];
				bestScoreArray.push_back(bestScore);
			}
		}
	}
// SORT picking the best move. NOT bestValue
	sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
	/*
	 if (debugThis) {
	 for (int ii = 0; ii < bestScoreArray.end() - bestScoreArray.begin();
	 ii++) {
	 cout << bestScoreArray[ii];
	 }
	 cout << endl;
	 }
	 */
	bestScore = bestScoreArray[0]; // BestScore
	widthAtDepth[depth] = MIN(width, (int )bestScoreArray.size()); // size of bestScoreArray can be smaller than "width"
#ifdef PRINTSCORE
	if (debugThis || debugScoringd || debugScoringAll || debugAllPaths
			|| (currentWidth == aDebug.debugWidthAtDepth[depth])) {
		printf("at depth = %d, at width=%d debugWidthatDept=%d", depth,
				currentWidth, aDebug.debugWidthAtDepth[depth]);
		cout << endl;

		foundPath = 0;
		printTrace();

		if (debugThis) {
			print(SYMBOLMODE);
			print(SCOREMODE);
		}
		for (int i = 0; i < widthAtDepth[depth]; i++) {
			returnScoreArray.push_back(bestScoreArray[i]); // initialize with prelim score

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
		if (debugThis) {
			do {
				cout << endl << "Enter # for debug" << endl;
				cin >> debugId;

				if (debugId < 0) {
					switch (debugId) {

					case -23:
						cout << "Turn " << FLIP(debugScoringd)
								<< " debugScoringd" << endl;

						break;
					case -24:
						cout << "Turn " << FLIP(debugScoringAll)
								<< " debugScoringAll" << endl;

						break;
					case -25:
						cout << "Turn " << FLIP(debugHash) << " debugHash"
								<< endl;

						break;

					case -3:
						cout << "Turn " << FLIP(debugScoring) << " debugScoring"
								<< endl;

						break;
					case -99:
						cout << "Turn " << FLIP(interactiveDebug)
								<< " interactiveDebug" << endl;
						break;
					case -13: {
						char ffn[80] = "savefile.txt";
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
					}

					default:
						break;
					}
				}
			} while (debugId < 0);
		}

		/*
		 if (debugScoringd || debugAllPaths)
		 prompt("hit enter");
		 */
	}
#endif
	// debug stuff

	if (foundPath == 0) {
		printf("Found debug depth d =%d, w = %d, %d", depth, currentWidth,
				aDebug.debugWidthAtDepth[depth]);
		cout << endl;
		if (depth == aDebug.debugBreakAtDepth) {
			cout << "Stopped at debugBreakAtDepth value d= " << depth << " "
					<< aDebug.debugBreakAtDepth;
			cout << endl;
			aDebug.enablePrinting = 0;
			prompt(" Hit Enter Key ");
		} else if (depth == (aDebug.debugBreakAtDepth + 1)) {
			cout << "Enable debug printing " << endl;
			aDebug.enablePrinting = 1;
		}
	}
// unless this is the last depth, playing the next hand (of the previous best fews)
// Recursively call to evaluate that play. The next call is for the opponent hand.
	parent.top.ptr = bestScoreArray[0].cellPtr;
	parent.top.width_id = 0;
	parent.top.depth_id = depth;
	parent.top.val = setVal;
	bestScore = bestScoreArray[0];
	/*
	 if (foundPath != -1234) {
	 if (terminated)
	 cout << "Terminated at d=" << depth << " w=" << currentWidth
	 << endl;
	 }
	 */
	if ((terminated == 0) && (depth > 0)) {
		if (maximizingPlayer)
			bestScore = dworstValue();
			else
			bestScore = dbestValue();
			breadCrumb myCrumb(depth - 1);

			for (int i = 0; i < widthAtDepth[depth]; i++) {
// TODO working on adding defense score to scoreElement
				/*
				 printf("d=%d,w=%d%c  ",depth,i,convertToChar(setVal));
				 if((depth-prevD)>3)
				 cout << endl;
				 */
				prevD = depth;
				cPtr = bestScoreArray[i].cellPtr;
				int saveVal = cPtr->val;
				setCell(setVal, cPtr->rowVal, cPtr->colVal, E_TNEAR);

				///////
				trace.push_back((unsigned char)i);

				redoNext = false;
				do {
					bool debugNext = (debugId == i);
					returnScore = evalAllCell(opnVal, width, depth - 1, i + foundPath,
							!maximizingPlayer, myCrumb,debugNext, redoNext);
				}while(redoNext);
				trace.pop_back();
				/*
				 if (foundPath == 0) {
				 returnScoreArray[i].score(returnScore);
				 }
				 */
				if (foundPath == 0) returnScoreArray[i] = returnScore;

				//bestScoreArray[i] = returnScore;
#ifdef PRINTSCORE
				if (currentWidth == aDebug.debugWidthAtDepth[depth]) {
					aDebug.Array[depth][i].ts_ret = returnScore;
				}
#endif
				bool found_better =
				maximizingPlayer ?
				//betterMove(returnScore,bestScore):
				//!betterMove(returnScore,bestScore);

				betterValue(returnScore,bestScore):
				betterValue(bestScore,returnScore);

				if (found_better) {
					if(debugThis) {
						printf("i=%d,",i);
						cout << "bestScore=" << bestScore << " Return=" << returnScore << endl;
					}
					bestScore = returnScore;
					bestScore.cellPtr = bestScoreArray[i].cellPtr;
					parent = myCrumb;
					parent.top.ptr = bestScoreArray[i].cellPtr;
					parent.top.width_id = i;
					//		prompt("found better");

				}
				restoreCell(saveVal, cPtr->rowVal, cPtr->colVal);
				if (bestScore.getScore(setVal,myVal) >= MAGICNUMBER) {
					// quit if setVal's score is MAGIC
					// cout << "Terminated" << endl;
					break;
				}

				/*
				 if (maximizingPlayer & (returnScore.val >= MAGICNUMBER)) {
				 //	cout <<"\n\t\t\tB"<<endl;
				 break;//  <<<<<<----------- critical
				 }
				 */
			}
// TODO scoring for opponent cause the MAX value, but not bc of winning!!! it is fair for calculating best move
// but for miniMax purpose. Dont want to return that value to calling function. ???
			if (foundPath == 0) {
				printTrace();

				if (maximizingPlayer)
				cout << "Max ";
				else
				cout << "Mini";
				for (int i = 0; i < widthAtDepth[depth]; i++) {
					cPtr = aDebug.Array[depth][i].cellPtr;
					if (debugThis||debugScoring || debugBestPath || debugScoringAll
							|| debugAllPaths) {
						printf("\t<<%d,%d>>", depth, i);
						printf("%C| ", convertCellToStr(setVal));
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

				if(debugThis) {
					print(SYMBOLMODE);
					print(SCOREMODE);
				}

			}

		}
		/*
		 if (setCellCnt == 0) {
		 print(SCOREMODE);
		 print(SYMBOLMODE);
		 }
		 */
		// TOOO: check return score is 2 or 1, and if bestScore is use for comparing
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
