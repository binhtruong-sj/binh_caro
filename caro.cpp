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
#define MAGICNUMBER 99999
#define PRINTSCORE 1
#define prompt() {cout << "Hit Enter Key" << endl; cin.get(); }

#include "caro.h"
int search_depth = 5;
int search_width = 5;
int debugWidthAtDepth[MAXDEPTH];
int bestWidthAtDepth[MAXDEPTH];
tsDebug aDebug, bestScoreDebug;
int gdebug = 0;
int debugScoring = 0;
int debugScoringE = 0;
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

void cell::print(int mode) {
	if (mode == 0) {
		printf("%2X", val);
	} else if (mode == SYMBOLMODE) {
		char ach = val == 0x1 ? 'X' : val == 0x2 ? 'O' : val == E_FAR ? '-' :
					val == 0xb ? 'b' : val == E_NEAR ? '\"' : '.';
		printf("%2c ", ach);
	} else {
		printf("%2x ", score);
	}
}

void cell::print() {
	cout << val;
}

caro::caro(int table_size) {
	size = table_size + 1;
// Setup pointer to ajacent cells, only from 1 to size-1
	terminate = 0;
	for (int w = 0; w < 40; w++)
		widthAtDepth[w] = 0;

	for (int row = 0; row <= size; row++) {
		for (int col = 0; col <= size; col++) {
			board[row][col].rowVal = row;
			board[row][col].colVal = col;
			if ((row == 0) || (col == 0) || (row == size) || (col == size)) {
				board[row][col].val = BOUNDARY;
				for (int i = 0; i < 8; i++)
					board[row][col].near_ptr[i] = &board[row][col];
			} else {
				board[row][col].val = E_FAR; // FAR: empty cell 5 away from any occupied cell
				board[row][col].near_ptr[West] = &board[row][col - 1];
				board[row][col].near_ptr[NWest] = &board[row - 1][col - 1];
				board[row][col].near_ptr[North] = &board[row - 1][col];
				board[row][col].near_ptr[NEast] = &board[row - 1][col + 1];

				board[row][col].near_ptr[East] = &board[row][col + 1];
				board[row][col].near_ptr[SEast] = &board[row + 1][col + 1];
				board[row][col].near_ptr[South] = &board[row + 1][col];
				board[row][col].near_ptr[SWest] = &board[row + 1][col - 1];
			}
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
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	val = board[row][col].val;

	//oppval = (board[row][col].val == X_)? O_ : X_;
	oppval = oppositeVal(board[row][col].val);

	cell *currCell = &board[row][col];
	cell *oriCell = currCell;
	cell *backoffCell;
#ifdef VERBOSE3
	cout << "\nscan direction "<<dir << endl;
#endif

	// Scan for O_ (assuming X_ turn)
	int found_opp = 0;
	for (int i = 0; i < SEARCH_DISTANCE; i++) {
		if (currCell->val & oppval) {
			found_opp = 1;
			break;
		}
		backoffCell = currCell;
		currCell = currCell->near_ptr[dir];
	}
	if (found_opp) {
		currCell = backoffCell;
	} else {
		// Did not find O_, switch back to ori and scan in reverse
		currCell = oriCell;
		dir = ReverseDirection(dir);
		for (int i = 0; i < SEARCH_DISTANCE; i++) {
			if (currCell->val & oppval) {
				break;
			}
			backoffCell = currCell;
			currCell = currCell->near_ptr[dir];
		}
		//currCell = currCell->near_ptr[ReverseDirection(dir)]; // backoff 1
		currCell = backoffCell;
	}

	// Now reverse direction
	dir = ReverseDirection(dir);

	aline.val = 0;
	aline.connected = 0;
	int save = 0;
	for (int i = 0; i < (SEARCH_DISTANCE * 2 - 1); i++) {
		aline.val = aline.val << 1;
		aline.cnt++;
		if (currCell->val == val) {
			bitcnt++;
			aline.val = aline.val | 0x1;
			aline.connected++;
		} else {
			if (aline.connected > save)
				save = aline.connected;
			aline.connected = 0;
		}
		currCell = currCell->near_ptr[dir];
		if (currCell->val & oppval) {
			bitcnt--; // with block, 3 is the samne as 2 with no block
			break;
		}
	}
	if (save > aline.connected)
		aline.connected = save;
	if (aline.cnt >= 5)
		aline.connected = bitcnt;
	else
		aline.connected = 0;
	return aline;
}

void Line::print() {
	printf("Line=0x%X Cnt=%d Score=%d", val, cnt, score);
	cout << endl;
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
		bitcnt = bitcnt + (tval & 1);
		tval = tval >> 1;
	}
	score = //(connected - 1)*CONTINUOUS_BIAS +
			(bitcnt * bitcnt * bitcnt * 5) / cnt;
	if (connected == 4)
		score <<= 1;
#ifdef HASH
	ahash.addEntry(val, cnt, score);
#endif
	return score;
}

void FourLines::print() {
	for (int dir = East; dir < West; dir++) {
		Xlines[dir].print();
	}
}

int caro::score1Cell(int setVal, int row, int col) {
	FourLines astar;
	astar.score = 0;
	int saveVal = board[row][col].val;
	int ill_6, ill_4, ill_3;
	ill_6 = ill_4 = ill_3 = 0;
	setCell(setVal, row, col, E_TNEAR);
	int multiples = 0;
	for (int dir = East; dir < West; dir++) {

		astar.Xlines[dir] = extractLine(dir, row, col);
		astar.score = astar.score + astar.Xlines[dir].evaluate();
		if (astar.Xlines[dir].evaluate())
			multiples++;
		if (astar.Xlines[dir].connected >= 6)
			ill_6 = 1;
		if (astar.Xlines[dir].connected == 3)
			ill_3++;
		if (astar.Xlines[dir].connected == 4)
			ill_4++;
	}
	if ((ill_6 || (ill_4 > 2) || (ill_3 > 2)) && setVal == X_)
		astar.score = -MAGICNUMBER;
	else
		astar.score = biasAdjust(myVal, setVal, astar.score, BIAS_PERCENT);
	if ((astar.score >= MAGICNUMBER) && (setVal != myVal)) {
		astar.score = -MAGICNUMBER;
	}
	board[row][col].score += astar.score * multiples;
	restoreCell(saveVal, row, col);
	return astar.score;
}
bool morecmp(scoreElement &s1, scoreElement &s2) {
	return (s1.val > s2.val);
}
scoreElement caro::evalAllCell(int setVal, int width, int depth,
		int currentWidth, breadCrumb &parent) {

	cell *cPtr;
	scoreElement bestScore;
	vector<scoreElement> aScoreArray;
	int opnVal = oppositeVal(setVal);
	int foundPath = -9999;
	int terminated = 0;
	clearScore();
//	cout << "eval, currentW=" << currentWidth << "debugwithatDepth="
//			<< debugWidthAtDepth[depth] << endl;
// Scoring all possible cells (nearby cells).
// Sorting the results, only keep a few best one (WIDTH).
	for (int row = 1; row < size; row++) {
		for (int col = 1; col < size; col++) {
			if (terminated)
				break;
			if (board[row][col].val & (E_NEAR | E_TNEAR)) {
				int returnVal = score1Cell(setVal, row, col);
				bestScore.val = returnVal;
				if (abs(returnVal) >= MAGICNUMBER) {
					terminated = 1; // not sure about this.  Terminate too early is foily. Keep on going, and calculate best position is better.
				} else {
					returnVal = bestScore.val + score1Cell(opnVal, row, col);
					bestScore.val = returnVal;
				}
				if (abs(returnVal) >= MAGICNUMBER) {
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
	bestWidthAtDepth[depth] = 0;
#ifdef PRINTSCORE
	if (currentWidth == debugWidthAtDepth[depth]) {
		foundPath = 0;
//		cout << "DEBUG at Depth " << depth << " at cw=" << currentWidth
//				<< ",dwad" << debugWidthAtDepth[depth] << endl;
		if(debugScoring) print(SCOREMODE);
		for (int i = 0; i < widthAtDepth[depth]; i++) {
			aDebug.Array[depth][i].val = aScoreArray[i].val;
			aDebug.Array[depth][i].cellPtr = aScoreArray[i].cellPtr;
			cPtr = aScoreArray[i].cellPtr;
			printf("<%d,%d>[%d%c]=$0x%x |", depth, i, cPtr->rowVal,
					convertToCol(cPtr->colVal), aScoreArray[i].val);
		}
		cout << endl;
	}
#endif
// unless this is the last depth, playing the next hand (of the previous best fews)
// Recursively call to evaluate that play. The next call is for the opponent hand.
	parent.top.ptr = aScoreArray[0].cellPtr;
	parent.top.width_id = 0;
	parent.top.depth_id = depth;

	if (depth > 0) {
		breadCrumb myCrumb(depth - 1);

		for (int i = 0; i < widthAtDepth[depth]; i++) {
			if (terminated)
				break;
			cPtr = aScoreArray[i].cellPtr;
			int saveVal = cPtr->val;
			setCell(setVal, cPtr->rowVal, cPtr->colVal, E_TNEAR);
			///////
//			cout <<"="<< *cPtr<<"=, depth=" <<
//						depth-1 << "width=" << width << "foundPath="<<foundPath << endl;

			scoreElement returnScore = evalAllCell(opnVal, width, depth - 1,
					i + foundPath, myCrumb);
			aDebug.Array[depth][i].ts_ret = returnScore.val;
//			printf("return Score=0x%x\n",returnScore.val);

#ifdef PRINTSCORE
			if (currentWidth == debugWidthAtDepth[depth]) {
				aDebug.Array[depth][i].ts_ret = returnScore.val;
				if (returnScore.val >= aScoreArray[0].val)
					bestWidthAtDepth[depth] = i;
			}
#endif
			if (returnScore.val >= bestScore.val) {
				bestScore.val = returnScore.val;
				bestScore.cellPtr = cPtr;
				parent = myCrumb;
				parent.top.ptr = aScoreArray[i].cellPtr;
				parent.top.width_id = i;
			}
			if (returnScore.val >= aScoreArray[0].val) {
				aScoreArray[0].val = returnScore.val;
			}
			restoreCell(saveVal, cPtr->rowVal, cPtr->colVal);
			if (bestScore.val >= MAGICNUMBER) { // only quit searching when find winner
				terminated = 1;
			}
		}
		if (foundPath >= 0) {
			for (int i = 0; i < widthAtDepth[depth]; i++) {
				cPtr = aScoreArray[i].cellPtr;
				printf("<<%d,%d>>[%d%c]=$0x%x |", depth, i, cPtr->rowVal,
						convertToCol(cPtr->colVal),
						aDebug.Array[depth][i].ts_ret);
			}
			cout << endl;
		}
	}

	if (setCellCnt == 0)
		print(SYMBOLMODE);
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
void hashTable::addEntry(int line, int bitcnt, int score) {
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
	if (i > 1000) {
		cout << "hashE cnt too small" << endl;
	} else {
		arrayE[arrayE_cnt].line = line;
		arrayE[arrayE_cnt].bitcnt = bitcnt;
		arrayE[arrayE_cnt].refcnt = 1;
		arrayE[arrayE_cnt++].score = score;

	}
}

void hashTable::print() {
	cout << "Hash Table: count = " << arrayE_cnt << "swapcnt =" << swapcnt
			<< endl;
	swapcnt = 0;
	printf("%4s %8s %8s %8s %8s %8s", "no", "Line", "bitcnt", "score", "RefCnt",
			"Binary");
	cout << endl;
	for (int i; i < arrayE_cnt; i++) {
		char binary[9];
		int val = arrayE[i].line;
		toBinary(val, binary);
		printf("%4d %8x %8d %8d %8d %8s\n", i, arrayE[i].line, arrayE[i].bitcnt,
				arrayE[i].score, arrayE[i].refcnt, binary);
	}
}
