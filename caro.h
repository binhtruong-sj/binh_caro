/*
 * caro.h
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */

#ifndef CARO_H_
#define CARO_H_
#define W 0
#define NW 1
#define N 2
#define NE 3
#define E 4
#define SE 5
#define S 6
#define SW 7

#define E_FAR 0 // cell val: empty, far from any occupied cell (4 too far)
#define BOUNDARY 0xb // for boundary cells
#define X_ 0x1    // occupied with X
#define O_ 0x2    // occupied with O
#define E_NEAR 0x4    // Empty cell, but close to other occupied cells
#define E_TNEAR 0x8    // for temporary Near, clear after move is made

#define SEARCH_DISTANCE 4
#define ReverseDirection(a) (a<4)? (a+4):(a-4)
/*
 * Line for the purpose of scoring
 */
class Line {
public:
	int val=0;
	int cnt=0;
	int score=0;
	int evaluate(Line aline);
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
	cell *near_ptr[8];
	void print();
};

/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */
class caro {
	cell board[21][21];
	int size = 17;
public:
	caro(int table_size);
	virtual ~caro();
	int setcell(int val, int x, int y);
	void setNEAR(int x, int y);
	void setTNEAR(int x, int y);
	void clearTNEAR(int x, int y);

	Line extractLine(int dir, int x, int y);
	int evalCell(int x, int y);
	void print();
};

#endif /* CARO_H_ */
