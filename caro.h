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

#define InspectDistance 3
#define SEARCH_DISTANCE 4
#define ReverseDirection(a) (a<4)? (a+4):(a-4)
//#define oppositeVal(a) a^0x3
#define oppositeVal(a) a==X_?O_:X_
#define myTurn(a) a%2
/*
 * Line for the purpose of scoring
 */
struct hEntry{
public:
	int line;
	int bitcnt;
	int score;
	int refcnt;
};
class hashTable{
public:
	hEntry arrayE[1000];
	int arrayE_cnt=0;
	hashTable();
	void addEntry(int line, int cnt, int score);
	void print();
};
class Line {
public:
	friend class hashTable;
	int val=0;
	int cnt=0;
	int score=0;
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
	int rowVal, colVal, score=0;
	cell *near_ptr[8];
	void print(int mode);
	void print();
};

struct scoreElement {
	int val;
	cell *cellPtr;
};

/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */
#define SYMBOLMODE 1
#define SCOREMODE 2

class caro {
	cell board[21][21];
	int size = 17;
	vector<scoreElement> aScoreArray;


public:
	caro(int table_size);
	virtual ~caro();
	int setCell(int val, int x, int y, int near);
	void setNEAR(int x, int y);
	void setTNEAR(int x, int y);
	void restoreCell(int val, int x, int y);
    void clearScore();
	Line extractLine(int dir, int x, int y);
	int score1Cell(int setVal, int row, int col);
	scoreElement evalAllCell(int val, int width, int depth);

	void print(int mode);
	void print();
};

#endif /* CARO_H_ */
