/*
 * caro.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <iostream>
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
#define MAGICNUMBER 99990
#define PRINTSCORE 1
#define prompt(a) {cout << a << endl; cin.get(); }
#define percentAdjust(val,percent) (val*percent)/128

#include "caro.h"
int search_depth = 5;
int search_width = 5;

int adjustCntArray[] = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128 };
int bitcntArray[] = { 0, 0, 16, 38, 300, 699, 999, 999, 999, 999, 999 };
#define BIAS_PERCENT 110  // 10% more than opponent
#define biasDefendAdjust(val) val*BIAS_PERCENT/128

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

cell * last2[256]; // fixed at 256, change to use remeainder if different setting
int last2v[256];
int last2p = 0;
int saveLast2p;
int moveCnt = 0;

tScore::tScore() {
	val = 0;
	ts_ret = 0;
	cellPtr = nullptr;
}

tScore::tScore(scoreElement a) {
	val = a.val;
	cellPtr = a.cellPtr;
	ts_ret = 0;
}

hashTable ahash;
tsDebug::tsDebug() {
	for (int i = 0; i < 40; i++) {
		for (int j = 0; j < 40; j++) {
			Array[i][j].val = Array[i][j].ts_ret = 0;
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
		for (int col = 0; col <= size; col++)
			board[row][col].score = 0;
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
		if (near == E_NEAR)
			setNEAR(row, col);
		else
			setTNEAR(row, col);
		return &board[row][col];
	} else
		return nullptr;
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */
void caro::setNEAR(int row, int col) {
	cell *currCell;
	for (int dir = East; dir <= SEast; dir++) {
		currCell = &board[row][col];
		for (int i = 0; i < InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			if ((currCell->val & 0x3) == 0) {
				currCell->val = E_NEAR;
			} else if ((currCell->val & 0x3) == 0x3)
				break;
		}
	}
}

/*
 * Temporary near marking set when traverse further ahead
 */
void caro::setTNEAR(int row, int col) {
	cell *currCell;
	for (int dir = East; dir <= SEast; dir++) {
		currCell = &board[row][col];
		for (int i = 0; i < InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			if ((currCell->val & 0x3) == 0x3)
				break;
			else if ((currCell->val & 0x7) == 0)
				currCell->val = E_TNEAR;
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
		for (int i = 0; i < 4; i++) {
			currCell = currCell->near_ptr[dir];
			if (currCell->val == E_TNEAR)
				currCell->val = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
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
Line caro::extractLine(int dir, int row, int col) {
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

	cell *currCell = &board[row][col];
	cell *oriCell = currCell;
	cell *backoffCell;	//, *lastone;
	//currCell = currCell->near_ptr[ReverseDirection(dir)];

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool prevVal = false;
	aline.type = val;
//	debugLine = true;

	if (debugLine) {
		cout << "Dir=" << dir << " myVal=" << myVal << " val=" << val;
		cout << endl << board[row][col] << " --> ";
	}
//	lastone = currCell;
	int spaced = 0;
	for (int i = 0; i < SEARCH_DISTANCE; i++) {
		if (debugLine) {
			currCell->printid();
			currCell->print(SYMBOLMODE);
		}
		prevVal = currCell->val == val;
		if (currCell->val & oppval) {
			if (prevVal & (currCell->val == oppval))
				aline.blocked = 1;
			break;
		} else if (currCell->val == val) {
			backoffCell = currCell;
		} else {
			if (spaced++ > 1)
				break;
		}
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

		for (int i = 0; i < SEARCH_DISTANCE; i++) {
			if (debugLine) {
				currCell->printid();
				currCell->print(SYMBOLMODE);
			}
			prevVal = currCell->val == val;
			if (currCell->val & oppval) {
				if (prevVal & (currCell->val == oppval))
					aline.blocked = 1;
				break;
			} else if (currCell->val == val) {
				backoffCell = currCell;
			} else {
				if (spaced++ > 1)
					break;
			}
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
		if (debugLine) {
			currCell->printid();
			currCell->print(SYMBOLMODE);
		}
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
	if ((aline.connected == 4) && (marked >= 1))
		aline.connected--;

	if (debugLine) {
		cout << endl << aline << endl;
	}
	if (save > aline.connected)
		aline.connected = save;
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt >= 5) {
		if (bitcnt <= 4)
			aline.connected -= aline.blocked;
	} else
		aline.connected = 0;

	if ((val == myVal) && (aline.connected > 0))
		aline.connected--; // go first, assume that opponent to block  (1 less)

	return aline;
}

int Line::evaluate() {
	// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
	// self learning)
	score = 0;
	int bitcnt = 0;
	if (cnt < 5)
		return score = 0;
	if (((connected >= 5) && (type == O_))
			|| ((connected == 5) && (type == X_))) {

		return MAGICNUMBER * 2;
	}
	int tval = val;
	while (tval) {
		bitcnt = bitcnt + (tval & 1); // recalculate bitcnt
		tval = tval >> 1;
	}
	/*
	 score = //(connected - 1)*CONTINUOUS_BIAS +
	 (bitcnt * bitcnt * bitcnt * 5) / cnt;
	 */
	score = (bitcntArray[connected] + (bitcnt - connected)) * 128;
	score = percentAdjust(score, adjustCntArray[bitcnt]);

	if (connected == 4)
		score *= 2;
#ifdef HASH
	ahash.addEntry(val, connected, cnt, score);
#endif
	return score;
}

void FourLines::print() {
	for (int dir = East; dir < West; dir++) {
		Xlines[dir].print();
	}
}

int caro::score1Cell(int setVal, int row, int col, int depth) {
	FourLines astar;
	astar.score = 0;
	int saveVal = board[row][col].val;
	int ill_6, ill_4, ill_3;
	ill_6 = ill_4 = ill_3 = 0;
	setCell(setVal, row, col, E_TNEAR);
	for (int dir = East; dir < West; dir++) {
		evalCnt++;
		astar.Xlines[dir] = extractLine(dir, row, col);
		int tscore = astar.Xlines[dir].evaluate();
		//	if (aDebug.enablePrinting||cdbg.ifMatch(&board[row][col],depth,0)) {
		if (aDebug.enablePrinting) {
			cout << board[row][col] << "dir=" << dir;
			cout << astar.Xlines[dir] << endl;
			if (aDebug.printCnt++ % 40 == 0)
				prompt("Hit Enter Key");
		}
		astar.score = astar.score + tscore;
		if (astar.Xlines[dir].connected >= 6)
			ill_6 = 1;
		if (astar.Xlines[dir].connected == 3)
			ill_3++;
		if (astar.Xlines[dir].connected == 4)
			ill_4++;
	}
	if ((ill_6 || (ill_4 > 2) || (ill_3 > 2)) && setVal == X_)
		astar.score = -MAGICNUMBER;
	//else
	//	astar.score = biasAdjust(myVal, setVal, astar.score, BIAS_PERCENT);

	board[row][col].score += astar.score;
	if (setVal == myVal)
		myMoveAccScore += astar.score;
	else
		opnMoveAccScore += astar.score;

	restoreCell(saveVal, row, col);

	// opponent is playing second, hence lower score
	if (setVal != myVal)
		astar.score = biasDefendAdjust(astar.score);
	return astar.score;
}
bool morecmp(scoreElement &s1, scoreElement &s2) {
	return (s1.val > s2.val);
}
/*
 *
 *
 */
scoreElement caro::evalAllCell(int setVal, int width, int depth,
		int currentWidth, bool maximizingPlayer, breadCrumb &parent) {

	cell *cPtr;
	scoreElement bestScore, returnScore;
	vector<scoreElement> aScoreArray, returnScoreArray;
	int opnVal = oppositeVal(setVal);
	int foundPath = -8888;
	int terminated = 0;
	clearScore();
	for (int row = 1; row < size; row++) {
		for (int col = 1; col < size; col++) {
			if (terminated)
				break;
			if (board[row][col].val & (E_NEAR | E_TNEAR)) {
				int returnVal = score1Cell(setVal, row, col, depth);
				bestScore.val = returnVal;
				if (abs(returnVal) >= MAGICNUMBER) {
					terminated = 1; // not sure about this.  Terminate too early is foily. Keep on going, and calculate best position is better.
				} else {
					returnVal = bestScore.val
							+ score1Cell(opnVal, row, col, depth);
					bestScore.val = returnVal;
				}
				if (abs(returnVal) >= MAGICNUMBER) {
					//			cout <<"\n\t\t\t\tb\n";

					terminated = 1;
				}
				bestScore.cellPtr = &board[row][col];
				aScoreArray.push_back(bestScore);
			}
		}
	}

// SORT
	sort(aScoreArray.begin(), aScoreArray.end(), morecmp);
	bestScore = aScoreArray[0]; // BestScore
	widthAtDepth[depth] = MIN(width, (int )aScoreArray.size()); // size of aScoreArray can be smaller than "width"
#ifdef PRINTSCORE
	if (debugScoringd || debugScoringAll || debugAllPaths
			|| (currentWidth == aDebug.debugWidthAtDepth[depth])) {
		printf("at depth = %d, at width=%d debugWidthatDept=%d", depth,
				currentWidth, aDebug.debugWidthAtDepth[depth]);
		cout << endl;
		foundPath = 0;
		for (int i = 0; i < widthAtDepth[depth]; i++) {
			returnScoreArray.push_back(aScoreArray[i]); // initialize with prelim score

			if (depth < aDebug.lowDepth)
				aDebug.lowDepth = depth;
			aDebug.Array[depth][i].val = aScoreArray[i].val;
			aDebug.Array[depth][i].cellPtr = aScoreArray[i].cellPtr;
			cPtr = aScoreArray[i].cellPtr;
			if (debugScoring || debugBestPath || debugScoringAll
					|| debugAllPaths) {
				printf("\t<%d,%d>", depth, i);
				cPtr->printid();
				printf("%C=$0x%x |", convertCellToStr(setVal),
						aScoreArray[i].val / 128);
				if (i % 2 == 0)
					printf("\n");
			}
		}
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
	parent.top.ptr = aScoreArray[0].cellPtr;
	parent.top.width_id = 0;
	parent.top.depth_id = depth;
	parent.top.val = setVal;
	bestScore = aScoreArray[0];
	if (foundPath != -1234) {
		if (terminated)
			cout << "Terminated at d=" << depth << " w=" << currentWidth
					<< endl;
	}
	if ((terminated == 0) && (depth > 0)) {
		if (maximizingPlayer)
			bestScore.val = -99999999;
		else
			bestScore.val = 99999999;
		breadCrumb myCrumb(depth - 1);
		for (int i = 0; i < widthAtDepth[depth]; i++) {

			/*
			 printf("d=%d,w=%d%c  ",depth,i,convertToChar(setVal));
			 if((depth-prevD)>3)
			 cout << endl;
			 */
			prevD = depth;
			cPtr = aScoreArray[i].cellPtr;
			int saveVal = cPtr->val;
			setCell(setVal, cPtr->rowVal, cPtr->colVal, E_TNEAR);

			///////
			returnScore = evalAllCell(opnVal, width, depth - 1, i + foundPath,
					!maximizingPlayer, myCrumb);
			if (debugScoringd || (foundPath == 0)) {
				printf("[%d,%d]", depth, i);
				print(SYMBOLMODE);
				print(SCOREMODE);
				if (debugScoringd || debugAllPaths)
					prompt("hit enter");
			}
			if (foundPath == 0) {
				returnScoreArray[i] = returnScore;
			}
			aScoreArray[i].val = returnScore.val;
#ifdef PRINTSCORE
			if (currentWidth == aDebug.debugWidthAtDepth[depth]) {
				aDebug.Array[depth][i].ts_ret = returnScore.val;
			}
#endif
			bool found_better =
					maximizingPlayer ?
							(returnScore.val >= bestScore.val) :
							(returnScore.val < bestScore.val);
			if (found_better) {
				//		printf("playing %d bs=%d , rs=%d\n", opnVal, bestScore.val,
				//				returnScore.val);
				bestScore.val = returnScore.val;
				bestScore.cellPtr = aScoreArray[i].cellPtr;
				parent = myCrumb;
				parent.top.ptr = aScoreArray[i].cellPtr;
				parent.top.width_id = i;
				//		prompt("found better");

			}
			restoreCell(saveVal, cPtr->rowVal, cPtr->colVal);
			/*	if (bestScore.val >= MAGICNUMBER) { // only quit searching when find winner
			 cout << "Terminated" << endl;
			 //break;

			 }
			 */
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
			if (maximizingPlayer)
				cout << "Max ";
			else
				cout << "Mini";
			for (int i = 0; i < widthAtDepth[depth]; i++) {
				cPtr = aDebug.Array[depth][i].cellPtr;
				if (debugScoring || debugBestPath || debugScoringAll
						|| debugAllPaths) {
					printf("\t<<%d,%d>>", depth, i);
					cPtr->printid();
					printf("%C| ", convertCellToStr(setVal));
					cout << returnScoreArray[i] << "\t";

					if (i % 2 == 0)
						printf("\n");
				}
			}
			cout << " bestScore=" << bestScore;
			cout << endl;
		}

	}
	/*
	 if (setCellCnt == 0) {
	 print(SCOREMODE);
	 print(SYMBOLMODE);
	 }
	 */
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
