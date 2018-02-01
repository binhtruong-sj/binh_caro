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
#define E_NEAR 0x4    // Empty cell, but close to other occupied cells
#define E_TNEAR 0x8    // for temporary Near, clear after move is made

#define printInterval 100000
#define MAXDEPTH 41
#define InspectDistance 3
#define SEARCH_DISTANCE 4
#define ReverseDirection(a) (a<4)? (a+4):(a-4)
//#define oppositeVal(a) a^0x3
#define oppositeVal(a) a==X_?O_:X_
#define myTurn(a) a%2
#define biasAdjust(favor_val, val,score,percent) (val == favor_val) ? ((score*percent)/100): score
#define BIAS_PERCENT 105
#define MAXWIDTH 50
#define CONTINUOUS_BIAS 1

#define SYMBOLMODE 1
#define SCOREMODE 2
#define toBinary(v,bianry) {int vvall = v;\
								for(int bit=7; bit>=0; bit--) {\
										binary[bit] = (vvall & 0x1) + '0';\
										vvall >>= 1;\
								}\
								binary[8]= 0;}
#define MIN(a,b) (a<b) ? a:b
#define convertToChar(setVal) (setVal == X_ ? 'X' : 'O')
#define convertToCol(col) ((char)(col-1+'a'))
/*
 * Line for the purpose of scoring
 */

struct hEntry {
public:
	int line;
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
	void addEntry(int line, int cnt, int score);
	void print();
};

class Line {
public:
	friend class hashTable;
	int val = 0;
	int cnt = 0;
	int connected;
	int type; // X_ or O_ are being search
	int score = 0;
	int evaluate();
	void print();
};

/*
 * composite score for all crosslines
 */
class FourLines {
public:
	Line Xlines[4];
	int score = 0;
	void print();
};

/*
 * a cell in a caro table, has pointers to adjacent cells
 */
class cell {
public:
	int val = 0;
	int rowVal, colVal, score = 0;
	cell *near_ptr[8];
	void print(int mode);
	void print();
	void printid() {
		printf("[%d%c]", rowVal, convertToCol(colVal));
	}
	friend ostream & operator <<(ostream &out, cell &c) {
		out << "[" << c.rowVal << convertToCol(c.colVal) << "]";
		return out;
	}
};

class scoreElement {
public:
	int val;
	cell *cellPtr = nullptr;
};

class tScore: public scoreElement {
public:
	int ts_ret;
	tScore();
	tScore(scoreElement);
};

class tsDebug {
public:
	tScore Array[40][40];
	tsDebug();
	void print(int maxdepth, int widthAtDepth[], int bestWidthAtDepth[]) {
		for (int d = maxdepth; d >= 0; d--) {
			cout << "Depth " << d << " Width=" << widthAtDepth[d] << " Best W:"
					<< bestWidthAtDepth[d] << endl;
			cout << "\t";
			for (int w = 0; w < widthAtDepth[d]; w++) {
				printf("<%d,%d>,[%d%c]=$0x%x,after$0x%x   ", d, w,
						Array[d][w].cellPtr->rowVal,
						convertToCol(Array[d][w].cellPtr->colVal),
						Array[d][w].val,
						Array[d][w].ts_ret);
			}
			cout << endl;
		}
	}
};
class acrumb {
public:
	int width_id, depth_id;
	cell *ptr = nullptr;
	friend ostream & operator <<(ostream &out, const acrumb &c) {
		out << "{w=" << c.width_id << " d=" << c.depth_id << " ";
		if (c.ptr)
			c.ptr->printid();
		else
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
			}
			array[dd] = other.top;
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
};

/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */

class caro {
public:
	cell board[21][21];
	int size = 17;
	int widthAtDepth[40];

	int terminate;
	int myVal = X_;
	caro(int table_size);
	virtual ~caro();
	unsigned setCellCnt = 0;
	cell* setCell(int val, int x, int y, int near);
	void setNEAR(int x, int y);
	void setTNEAR(int x, int y);
	cell* restoreCell(int val, int x, int y);
	void undo1move();
	void redo1move();

	void clearScore();
	Line extractLine(int dir, int x, int y);
	int score1Cell(int setVal, int row, int col);
	scoreElement evalAllCell(int val, int width, int depth, int currentWidth,
			breadCrumb &b);
	scoreElement terminateScore;
	void reset();
	void print(int mode);
	void print();
};

#endif /* CARO_H_ */
