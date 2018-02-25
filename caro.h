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

#define SCOREMODE 4

#define SCOREMODEm 8

#define toBinary(v,bianry) {int vvall = v;\
								for(int bit=7; bit>=0; bit--) {\
										binary[bit] = (vvall & 0x1) + '0';\
										vvall >>= 1;\
								}\
								binary[8]= 0;}
#define MIN(a,b) (a<b) ? a:b
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
	int type; // X_ or O_ are being search
	friend ostream & operator <<(ostream & out, Line & v) {
		char binary[9];
		toBinary(v.val, binary);
		out << "Line=" << binary << " Cnt=" << v.cnt;
		out << " Blocked=" << v.blocked << " Connected=" << v.connected;
		out << " Score = " << v.score;
		return out;
	}

	int evaluate();
	void print() {
		char binary[9];
		toBinary(val, binary);
		printf("Line=%s Cnt=%d blocked=%d connected =%d", binary, cnt, blocked,
				connected);
		cout << score;
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
			if ((score.val + score.defVal) > 0)
				printf("%3x ", score.val + score.defVal);
			else
				printf("    ");
			return;
		}
		case SYMBOLMODE2: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE2 + 1): {
			if ((score.val - score.defVal) != 0)
				printf("%3x ", score.val - score.defVal);
			else
				printf("    ");
			return;
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
				/*
				 printf("=$0x%x,ret$0x%x ", Array[d][w].val,
				 Array[d][w].ts_ret  );
				 */
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

class caro {
public:
	vector<unsigned char> trace;
	void printTrace() {
		cout << "TRACE: ";
		for (unsigned int i = 0; i < trace.size(); i++)
			printf("%d,", trace[i]);
		cout << endl;
	}
	cell board[21][21];
	int scorecnt, skipcnt;
	int prevD;
	int evalCnt = 0;
	int myMoveAccScore = 0;
	int opnMoveAccScore = 0;
	int size = 17;
	int widthAtDepth[40];

	int terminate;
	int myVal = X_;
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
	cell* setCell(int val, int x, int y, int near);
	void setNEAR(int x, int y,int near);
	void saveScoreVal(int row, int col,
			aScore saveScoreArray[8][InspectDistance], aScore & s1Score,
			int saveValArray[8][InspectDistance], int & s1Val) {
		cell *currCell;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				currCell = currCell->near_ptr[dir];
				saveScoreArray[dir][i] = currCell->score;
				saveValArray[dir][i] = currCell->val;

				if ((currCell->val & 0x3) == 0x3) // boundary
					break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
			aScore saveScoreArray[8][InspectDistance], aScore & s1Score,
			int saveValArray[8][InspectDistance], int & s1Val) {
		cell *currCell;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				currCell = currCell->near_ptr[dir];
				currCell->score = saveScoreArray[dir][i];
				currCell->val = saveValArray[dir][i];
				if ((currCell->val & 0x3) == 0x3) // boundary
					break;
			}
		}
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
	}
	cell* restoreCell(int val, int x, int y);
	void undo1move();
	void redo1move();

	void clearScore();
	Line extractLine(int inVal, int dir, int x, int y);
	aScore score1Cell(int setVal, int row, int col, int depth);
	scoreElement evalAllCell(int val, int width, int depth, int currentWidth,
			bool maximizingPlayer, breadCrumb &b, bool debugThis, bool &redo);
	scoreElement terminateScore;
	cellDebug cdbg;
	void reset();
};

#endif /* CARO_H_ */
