/*
 * caro.h
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <vector>
#include <algorithm>
#ifndef CARO_H_
#define CARO_H_

#define SEast 7
#define South 6
#define SWest 5
#define West 4
#define NWest 3
#define North 2
#define NEast 1
#define East 0

#define E_FAR 0 // cell val: empty, far from any occupied cell (4 too far)
#define BOUNDARY 0xb // for boundary cells
#define X_ 0x1    // occupied with X
#define O_ 0x2    // occupied with O
#define E_NEAR 0x10    // Empty cell, but close to other occupied cells
#define E_TNEAR 0x20    // for temporary Near, clear after move is made
#define E_CAL 0x100

#define printInterval 100000
#define MAXDEPTH 41
#define InspectDistance 3
#define saveRestoreDist InspectDistance+3
#define SEARCH_DISTANCE 7
#define ReverseDirection(a) (a+4)%8
//#define oppositeVal(a) a^0x3
#define oppositeVal(a) a==X_?O_:X_
#define myTurn(a) a%2
//#define biasAdjust(favor_val, val,score,percent) (val == favor_val) ? ((score*percent)/100): score
#define FLIP(a) ((a=a^1)? "ON":"OFF")

#define MAXWIDTH 50
#define CONTINUOUS_BIAS 1

#define SYMBOLMODE 0
#define SYMBOLMODE2 2
#define SYMBOLMODE3 4

#define SCOREMODE 6

#define SCOREMODEm 8

#define toBinary(v,bianry) {int vvall = v;\
								for(int bit=7; bit>=0; bit--) {\
										binary[bit] = (vvall & 0x1) + '0';\
										vvall >>= 1;\
								}\
								binary[8]= 0;}
#define MIN(a,b) (a<b) ? a:b
#define MAX(a,b) (a>b) ? a:b
#define convertToChar(setVal) (setVal == X_ ? 'X' : 'O')
#define convertToCol(col) ((char)(col-1+'a'))
#define convertCellToStr(val) (char) ((val == X_) ? 'X' : \
                              (val == O_) ? 'O' :\
		                   (val == E_FAR) ? '.' :\
		                     (val == 0xb) ? 'b' :\
		                     ( val & E_CAL)? '=':\
                          (val & E_TNEAR) ? '+' :\
                                              '*')

/*
 * Line for the purpose of scoring
 */
using std::hex;
using std::showbase;
struct hEntry {
public:
	int line;
	int connected;
	int bitcnt;
	int score;
	unsigned refcnt = 0;
};
class hashTable {
public:
	hEntry arrayE[1000];
	int arrayE_cnt = 0;
	unsigned lowest;
	int lowestI;
	int swapcnt = 0;
	hashTable();
	void swap2e(int from, int to);
	void addEntry(int line, int connected, int cnt, int score);
	void print();
};
class aScore {
public:
	int val, defVal;
	bool operator ==(const aScore & v) {
		bool out;
		out = (v.val == val) && (v.defVal == defVal);
		return out;
	}
	friend ostream & operator <<(ostream & out, const aScore & v) {
		out << "(" << hex << v.val << "." << v.defVal << ")";
		return out;
	}
	/*
	 aScore & operator =(const aScore & v) {
	 val = v.val;
	 defVal = v.defVal;
	 return *this;
	 }
	 */
	aScore & operator +=(const aScore & v) {
		val = val + v.val;
		defVal = defVal + v.defVal;
		return *this;
	}
	aScore & init(int v, int d) {
		val = v;
		defVal = d;
		return *this;
	}

	int bMove() {
		return val + defVal;
	}
	int bValue() {
		return val - defVal;
	}

};
class Line {
public:
	friend class hashTable;
	int score;
	int val = 0; // 32 bit encoded of a line
				 // not the whole line.  Just started on O_
				 // scan to maximum of 7 squares
	int cnt = 0; // how many X_
	int connected; // How long is the longest connected X_
	int blocked; // is it blocked by O_ (next to an X_).
	int offset; // to add or subtract to scoring
	int type; // X_ or O_ are being search
	friend ostream & operator <<(ostream & out, Line & v) {
		char binary[9];
		toBinary(v.val, binary);
		out << "Line=" << binary << " Cnt=" << v.cnt;
		out << " Blocked=" << v.blocked << " Connected=" << v.connected;
		out << " Score = " << hex << v.score;
		return out;
	}

	int evaluate(bool ending);
	void print() {
		char binary[9];
		toBinary(val, binary);
		printf("Line=%s Cnt=%d blocked=%d connected =%d", binary, cnt, blocked,
				connected);
		cout << hex << score;
		cout << endl;
	}
};

/*
 * composite score for all crosslines
 */
class FourLines {
public:
	Line Xlines[4];
	int score;
	void print();
};

/*
 * a cell in a caro table, has pointers to adjacent cells
 */
class cellV {
public:
	int val = 0;
	aScore score;
	friend ostream & operator <<(ostream & out, cellV& v) {
		out << convertCellToStr(v.val);
		return out;
	}
};
class cell: public cellV {
public:
	int rowVal, colVal;
	cell *near_ptr[8];
	void print() {
		cout << val;
	}

	void print(int v, int num) {
		if (val & (X_ | O_)) {
			char ach = (char) convertCellToStr(val);
			if (num < 0)
				printf("%3c ", ach);
			else
				printf("%2d%c ", num, ach);
		} else {
			char ach = (char) convertCellToStr(v);
			if (num < 0)
				printf("%3c ", '-');
			else
				printf("%2d%c ", num, ach);
		}
		return;
	}

	void print(int mode) {
		int pval;
		if (val & (X_ | O_)) {
			if ((mode % 2) == 0) {
				char ach = (char) convertCellToStr(val);
				printf("%3c ", ach);
			} else {
				printf("%3c ", ' ');
			}
			return;
		}

		switch (mode) {
		case SYMBOLMODE: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE + 1): {
			return;
		}
		case SYMBOLMODE2: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE2 + 1): {
			pval = score.val + score.defVal;
			break;
		}
		case SYMBOLMODE3: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE3 + 1): {
			pval = score.val - score.defVal;
			break;
		}
		case SCOREMODE: // myVal
			pval = score.val;
			break;
		case SCOREMODE + 1: // opnVal or defVal
			pval = score.defVal;
			break;
		case SCOREMODEm: // +
			pval = score.val + score.defVal;
			break;
		case SCOREMODEm + 1: // -
			pval = score.val - score.defVal;
			break;
		default:
			break;
		}
		if (pval)
			if (pval > 0xFFF)
				cout << "FFF ";
			else
				printf("%3x ", (0xFFF & (pval)));
		else {
			if ((mode % 2) == 0)
				printf("  . ");
			else
				printf("    ");
//TODO: y defval printed on X 9j
		}
	}

	void printid() {
		printf("[%d%c]", rowVal, convertToCol(colVal));
	}

	friend ostream & operator <<(ostream &out, cell &c) {
		out << "[" << dec << c.rowVal
				<< convertToCol(c.colVal) << "]" << convertCellToStr(c.val);
		return out;
	}

	bool operator ==(cell &lookUpCell) {
		/*
		 printf("Comparing Cell ");
		 lookUpCell.printid();
		 cout << "===" ;
		 printid();
		 */
		return ((lookUpCell.rowVal == rowVal) && (lookUpCell.colVal == colVal));
	}
};

class scoreElement: public aScore {
public:
	cell *cellPtr = nullptr;
	friend ostream & operator <<(ostream &output,
			vector<scoreElement> &values) {
		for (auto const& value : values) {
			output << *value.cellPtr << value.val << "," << value.defVal
					<< endl;
		}
		return output;

	}
	friend ostream & operator <<(ostream &out, scoreElement &c) {
		out << *c.cellPtr;
		out << hex << (c.val) << "," << hex << (c.defVal) << " ";
		return out;
	}
	scoreElement & operator=(const scoreElement & other) {
		val = other.val;
		defVal = other.defVal;
		cellPtr = other.cellPtr;
		return *this;
	}
	scoreElement & operator=(const aScore & other) {
		val = other.val;
		defVal = other.defVal;
		return *this;
	}
	scoreElement & operator=(cell & other) {
		cellPtr = &other;
		return *this;
	}
	scoreElement & score(scoreElement& f) {
		val = f.val;
		defVal = f.defVal;
		return *this;
	}
	inline int bestMove() {
		return val + defVal;
	}
	inline int bestValue() {
		return val - defVal;
	}
	bool greaterMove(scoreElement &other) {
		return (bestMove() > other.bestMove());
	}
	bool greaterValue(scoreElement &other) {
		return (bestValue() > other.bestValue());
	}
	int getScore(int setVal, int myval) { // returning setVal's score, need myVal for ref
		return ((setVal == myval) ? val : defVal);
	}
};

class tScore: public scoreElement {
public:
	aScore ts_ret;
	tScore();
	tScore(scoreElement);
};

class hist {
public:
	cell *hArray[100];
	int size;
	int gameCh;
	int locate(int row, int col) {
		for (int i = 0; i < size; i++) {
			if ((row == hArray[i]->rowVal) && (col == hArray[i]->colVal))
				return (i);
		}
		return (-1);
	}
};
class tsDebug {
public:
	tScore Array[40][40];
	int enablePrinting = 0;
	int printCnt = 0;
	int debugWidthAtDepth[MAXDEPTH];
	int debugBreakAtDepth = -1; // not breaking
// indicating the depth to take a break if follow
// the path of debugWidthAtDepth array

	int bestWidthAtDepth[MAXDEPTH];
	int lowDepth = 0;
	tsDebug();

	void print(int maxdepth, int widthAtDepth[], int bestWidthAtDepth[]) {
		for (int d = maxdepth; d >= lowDepth; d--) {
			cout << "Depth " << d << " Width=" << widthAtDepth[d] << " Best W:"
					<< bestWidthAtDepth[d];
			for (int w = 0; w < widthAtDepth[d]; w++) {
				if (w % 2 == 0)
					printf("\n");
				cout << "\t<" << d << "," << w << ">";
				cout << *Array[d][w].cellPtr;
				cout << "=$" << Array[d][w] << ",ret" << Array[d][w].ts_ret;
			}
			cout << endl;
		}
	}
};
class acrumb: public cellV {
public:
	int width_id, depth_id;

	cell *ptr = nullptr;
	friend ostream & operator <<(ostream &out, const acrumb &c) {
		out << "{w=" << c.width_id << " d=" << c.depth_id << " ";
		if (c.ptr) {
			c.ptr->printid();
			out << convertCellToStr(c.val);
		} else
			out << "NULL ";
		cout << "}";
		return out;
	}
};
class breadCrumb {
public:
	acrumb top;
	acrumb *array;
	breadCrumb(int depth) {
		top.depth_id = depth;
		array = new acrumb[depth];
	}
	~breadCrumb() {
		delete array;
	}
	friend ostream & operator <<(ostream &out, const breadCrumb&c) {
		out << c.top << " | ";
		for (int i = c.top.depth_id - 1; i >= 0; i--) {
			out << c.array[i] << " - ";
		}
		out << endl;
		return out;
	}

	void extractTohistArray(hist & histArray) {
		int j = 0;
		histArray.gameCh = top.val;
		histArray.hArray[j++] = top.ptr;
		for (int i = top.depth_id - 1; i >= 0; i--) {
			histArray.hArray[j++] = array[i].ptr;
		}
		histArray.size = j;
	}

	breadCrumb & operator=(const breadCrumb & other) {
		if (other.top.depth_id == top.depth_id) {
			top = other.top;
		} else if (other.top.depth_id <= top.depth_id - 1) {
			int dd =
					(other.top.depth_id < top.depth_id) ?
							other.top.depth_id : top.depth_id;
			for (int i = 0; i < dd; i++) {
				array[i] = other.array[i];
//				cout << "cc" << array[i];
			}
			array[dd] = other.top;
//			cout << "crumb=" << array[dd] << endl;
		} else {
			cout << "ERRROOOOROOOOR" << endl;
		}
		return *this;
	}
	int bestWidthAtDepth(int depth) {
		if (depth == top.depth_id)
			return top.width_id;
		else
			return array[depth].width_id;
	}
	cell * bestCellAtDepth(int depth) {
		if (depth == top.depth_id)
			return top.ptr;
		else
			return array[depth].ptr;
	}
};
class cellDebug {
	cell *debugCell[40];
	int dcDepth[40];
	int dcDline[40];
	int debugCnt = 0;

public:
	void reset() {
		debugCnt = 0;
	}
	void add(cell * dcell, int depth, int debugLine) {
		if (dcell) {
			cout << "add entry to cdb cnt=" << debugCnt << *dcell << endl;

			debugCell[debugCnt] = dcell;
			dcDepth[debugCnt] = depth;
			dcDline[debugCnt] = debugLine;
			debugCnt++;
		}
	}
	bool ifMatch(cell * lookupCell, int depth, int dcl) {
		for (int i = 0; i < debugCnt; i++) {
			if (depth < 0) {
				if ((dcl == dcDline[i]) && (*lookupCell == *debugCell[i]))
					return true;
			} else if ((dcl == dcDline[i]) && (depth == dcDepth[i])
					&& (*lookupCell == *debugCell[i]))
				return true;
		}
		return false;
	}
};
/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */
class caroDebug {
	vector<unsigned char> debugTrace, trace;
	int drow, dcol;
	/*
	 *
	 */
public:

	void printTrace() {
		cout << "\nTRACE: ";
		for (unsigned int i = 0; i < trace.size(); i++)
			printf("%d,", trace[i]);
	}
	/*
	 *
	 */
	void enterDebugTrace(char * debugString) {
		char *dstr;
		dstr = debugString;
		debugTrace.clear();
		while (char achar = *dstr++) {
			if (achar != ',') {
				debugTrace.push_back((unsigned int) (achar - '0'));
			}
		}
	}
	void enterDebugCell(char *debugString) {
		char ccol;
		sscanf(debugString, "[%d%c]", &drow, &ccol);
		if (isupper(ccol))
			dcol = ccol - 'A';
		else
			dcol = ccol - 'a';

		cout << "Debug info, row =" << drow << " col=" << dcol << endl;
	}
	bool traceMatch(int r, int c) {
		bool cellmatch = false;
		if ((drow >= 0) && (dcol >= 0)) {
			if ((r == drow) && (c == dcol))
				cellmatch = true;
		} else {
			cellmatch = true;
		}

		if (cellmatch) {
			if (trace == debugTrace) {
				cout << "---------------TRACE MATCH  row=" << r << " col=" << c
						<< endl;
				return true;
			}
		}

		return false;
	}
	void tracePush(int i) {
		trace.push_back((unsigned int) i);
	}

	void tracePop() {
		return (trace.pop_back());
	}

}
;

class caro {
public:
	caroDebug cdebug;
	vector<unsigned char> trace;
	void printTrace() {
		cout << "\nTRACE: ";
		for (unsigned int i = 0; i < trace.size(); i++)
			printf("%d,", trace[i]);
	}
	cell board[21][21];
	cell *lastCell, *last2Cell;
	int lastCellType, last2CellType;
	int scorecnt, skipcnt;
	int prevD;
	int evalCnt = 0;
	int myMoveAccScore = 0;
	int opnMoveAccScore = 0;
	int size = 17;
	int widthAtDepth[40];

	int terminate;
	int myVal = X_;
	cell * last2[256]; // fixed at 256, change to use remeainder if different setting
	int last2v[256];
	int last2p = 0;
	int saveLast2p;
	int moveCnt = 0;
	caro(int table_size) {
		size = table_size + 1;
		// Setup pointer to ajacent cells, only from 1 to size-1
		terminate = 0;
		for (int w = 0; w < 40; w++)
			widthAtDepth[w] = 0;

		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col].rowVal = row;
				board[row][col].colVal = col;
				if ((row == 0) || (col == 0) || (row == size)
						|| (col == size)) {
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
	;

	virtual ~caro();
	friend ostream & operator <<(ostream &out, const caro &c) {
		out << endl;
		for (int row = 0; row <= c.size; row++) {
			for (int col = 0; col <= c.size; col++) {
				out << " " << (char) convertCellToStr(c.board[row][col].val);
			}
			out << " ROW " << row << endl;
		}
		out << "  ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			out << " " << pchar;
		out << endl;
		return out;
	}

	void print(int mode) {
		cout << "MODE=" << mode << " skipcnt=" << skipcnt << " scorecnt="
				<< scorecnt << endl;

		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col].print(mode);
			}
			cout << " ROW " << dec << row << endl;
			for (int col = 0; col <= size; col++) {
				board[row][col].print(mode + 1);
			}

			cout << endl;
		}
		cout << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%3C ", pchar);
		cout << endl;
		printf("evalCnt=%d myScore=%d opnScore=%d delta=%d", evalCnt,
				myMoveAccScore, opnMoveAccScore,
				opnMoveAccScore - myMoveAccScore);
		cout << endl;
	}

	void print(hist &histArray) {
		int val = histArray.gameCh;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				int lh = histArray.locate(row, col);
				val = oppositeVal(val);
				board[row][col].print(val, lh);
			}
			cout << " ROW " << dec << row << endl;
			cout << endl;
		}
		cout << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%3C ", pchar);
		cout << endl;
	}

	void print() {
		cout << endl;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col].print(0);
			}
			cout << " ROW " << row << endl;
		}
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%2C ", pchar);
		cout << endl;
	}
	unsigned setCellCnt = 0;
	/*
	 * set a cell to X or O
	 */
	cell * setCell(int setVal, int row, int col, int near) {
		if ((board[row][col].val & (X_ | O_)) == 0) { // this check is bc of lazyness of setting up board
			if (setCellCnt++ == printInterval) {
				setCellCnt = 0;
			}
			if (near == E_NEAR) { // Only real set can be UNDO
				evalCnt = 0;
				myMoveAccScore = 0;
				opnMoveAccScore = 0;
				last2p = (last2p + 1) & 0xFF;
				saveLast2p = last2p;
				last2[last2p] = &board[row][col];
				last2v[last2p] = board[row][col].val;
			}
			board[row][col].val = setVal;
			board[row][col].score = {0,0};
			if (near != E_FAR) {
				last2Cell = lastCell;
				last2CellType = lastCellType;
				lastCell = &board[row][col];
				lastCellType = near;
				setNEAR(row, col, near);
			}
		}
		return &board[row][col];

	}

	/*
	 * Temporary near marking set when traverse further ahead
	 */
	void setNEAR(int row, int col, int near) {
		cell *currCell;
		int rval;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				currCell->val = currCell->val & ~(E_CAL);
				if ((currCell->val & (O_ | X_ | E_NEAR)) == 0)
					currCell->val = near;
				if (rval == 3) // boundary
					break;
			}
		}
	}

	/*
	 * After temporary setting a Cell to X' or O' for scoring, now return it and its neighbor to prev values
	 */
	cell * restoreCell(int saveVal, int row, int col) {
		cell *currCell;
		int rval;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
						i = 0;

				} while ((rval != 3) && rval);
				if (currCell->val & (E_TNEAR)) {
					currCell->val = currCell->val & ~(E_CAL);
					currCell->score= {0,0};
					currCell->val = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
				} else if (currCell->val & (E_NEAR)) {
					currCell->val = currCell->val & ~(E_CAL);
					currCell->score= {0,0};
				} else if (rval == 3) // boundary
				break;
			}
		}
		board[row][col].val = saveVal & ~(E_CAL); // Return the val to prev
		return currCell;
	}
	void saveScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		cell *currCell;
		int rval;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < saveRestoreDist; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
				} while ((rval != 3) && rval);
				saveScoreArray[dir][i] = currCell->score;
				saveValArray[dir][i] = currCell->val;
				if (rval == 3) // boundary
					break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		cell *currCell;
		int rval;
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < saveRestoreDist; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
				} while ((rval != 3) && rval);
				currCell->score = saveScoreArray[dir][i];
				currCell->val = saveValArray[dir][i];
				if (rval == 3) // boundary
					break;
			}
		}
	}

	class traceCell {
	public:
		cell *cell;
		traceCell *prev;
	};
	void printDebugInfo(int row, int col, traceCell * trace) {
		printTrace();
		do {
			if (trace->cell)
				cout << *(trace->cell) << "->";
			trace = trace->prev;
		} while (trace);
		cout << endl;
	}
	void modifyDebugFeatures(int a);
	void undo1move();
	void redo1move();
	cell * inputCell() {
		int row, col;
		char ccol, inputstr[20];
		bool invalid = true;
		do {
			cout << "Enter cell:";
			cin >> inputstr;
			sscanf(inputstr, "%d%c", &row, &ccol);
			col = ccol - 'a' + 1;
			if (((row > 0) && (row < 16)) && ((col > 0) && (col < 16))) {
				return (&board[row][col]);
			}
		} while (invalid);
	}
	void clearScore();
	Line extractLine(int inVal, int dir, int x, int y, bool &ending,
			bool debugThis);
	aScore score1Cell(int setVal, int row, int col, int depth, bool debugThis);
	scoreElement evalAllCell(int val, int width, int depth, int currentWidth,
			bool maximizingPlayer, int alpha, int beta, breadCrumb &b,
			bool debugThis, bool &redo, traceCell * trace);
	scoreElement terminateScore;
	cellDebug cdbg;
	int nextWidth(int depth, int width) {
		int nw = width - (width / (depth + 1));
		if (nw <= 0)
			nw = 1;
		return (nw);
	}
	void reset();
};

#endif /* CARO_H_ */
