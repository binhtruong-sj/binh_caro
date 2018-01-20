/*
 * caro.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <iostream>
using namespace std;

#include "caro.h"

void cell::print(){
	char ach = 	val==0x1 ? 'X':
				val==0x2 ? 'O':
				val==E_FAR ? '-':
				val==0xb ? 'b':
				val==E_NEAR ? '?': '.';
	cout << ach;
}
caro::caro(int table_size) {
	size = table_size+1;
// Setup pointer to ajacent cells, only from 1 to size-1
	for(int row=0; row<=size; row++){
		for(int col=0; col<=size; col++){
			if((row==0)||(col==0)||(row==size)||(col==size)) {
				board[row][col].val = BOUNDARY;
			}else{
				board[row][col].val = E_FAR; // FAR: empty cell 5 away from any occupied cell
				board[row][col].near_ptr[W] = &board[row][col-1];
				board[row][col].near_ptr[NW] = &board[row-1][col-1];
				board[row][col].near_ptr[N] = &board[row-1][col];
				board[row][col].near_ptr[NE] = &board[row-1][col+1];

				board[row][col].near_ptr[E] = &board[row][col+1];
				board[row][col].near_ptr[SE] = &board[row+1][col+1];
				board[row][col].near_ptr[S] = &board[row+1][col];
				board[row][col].near_ptr[SW] = &board[row+1][col-1];
			}
		}
	}
}
caro::~caro() {
	// TODO Auto-generated destructor stub
}

/*
 * print out the entire table -- only for text base, new routine is needed for GUI
 */
void caro::print() {
    cout << endl;
	for(int row=0; row<=size; row++){
		for(int col=0; col<=size; col++){
			board[row][col].print();
		}
		cout << " ROW " << row << endl;
	}
	cout << "0ABCDEFGHIJKLMNOP" << endl;
}

/*
 * set a cell to X or O
 */
int caro::setcell(int val, int row, int col) {
	if((board[row][col].val &0x3) == 0) {
		board[row][col].val = val;
		setNEAR(row,col);
		return 0;
	}
	else
		return 1;
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */
void caro::setNEAR(int row,int col) {
	cell *curr_cell;
	for(int dir = 0; dir <8; dir++) {
		curr_cell = &board[row][col];
		for(int i = 0;i <4; i++) {
			curr_cell = curr_cell->near_ptr[dir];
			if((curr_cell->val & 0x3)==0) {
				curr_cell->val = E_NEAR;
			}else if((curr_cell->val & 0x3)==0x3)
				break;
		}
	}
}

/*
 * Temporary near marking set when traverse further ahead
 */
void caro::setTNEAR(int row,int col) {
	cell *curr_cell;
	for(int dir = 0; dir <8; dir++) {
		curr_cell = &board[row][col];
		for(int i = 0;i <4; i++) {
			curr_cell = curr_cell->near_ptr[dir];
			if((curr_cell->val & 0x3)==0x3)
				break;
			else if((curr_cell->val & 0x7)==0)
				curr_cell->val = E_TNEAR;
		}
	}
}
/*
 * For time saving purpose, clear on the Temp
 */
void caro::clearTNEAR(int row,int col) {
	cell *curr_cell;
	for(int dir = 0; dir <8; dir++) {
		curr_cell = &board[row][col];
		for(int i = 0;i <4; i++) {
			curr_cell = curr_cell->near_ptr[dir];
			if(curr_cell->val==0x8) curr_cell->val = E_FAR;
		}
	}
}
/*
 * extracting a line for the purpose of scoring
 * This will need to work with how scoring is done.  Extracting will need to go
 * in hand.  So, this will need to change depend on the method
 * ONGOING CODING -- NOT DONE
 */
Line caro:: extractLine(int dir, int row, int col) {
	Line aline;
	aline.val = 0; aline.cnt = 0;
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	val = board[row][col].val ;
	cout << endl << "_________________dir=" << dir << " r=" << row << " c="<< col << " val=" << val << endl;

	oppval = (board[row][col].val == X_)? O_ : X_;
	cell *curr_cell = &board[row][col];
	cell *ori_cell = curr_cell;

	cout << "\nscan direction "<<dir << endl;
	// Scan for O_ (assuming X_ turn)
	int found_opp = 0;
	for(int i=0;i<SEARCH_DISTANCE;i++) {
		curr_cell->print();
		cout <<",";
		if(curr_cell->val&oppval) {
			found_opp = 1;
			break;
		}
		curr_cell = curr_cell->near_ptr[dir];
	}
	if(found_opp) {
		curr_cell = curr_cell->near_ptr[ReverseDirection(dir)]; // backoff 1
	} else {
		// Did not find O_, switch back to ori ad scan in reverse direction
		curr_cell = ori_cell;
		dir = ReverseDirection(dir);
		cout << "\nscan direction "<<dir << endl;

		for(int i=0;i<SEARCH_DISTANCE;i++) {
			curr_cell->print();
			cout <<",";
			if(curr_cell->val&oppval) {
				break;
			}
			curr_cell = curr_cell->near_ptr[dir];
		}
		curr_cell = curr_cell->near_ptr[ReverseDirection(dir)]; // backoff 1
	}

	cout << endl;
	// Now reverse direction
	dir = ReverseDirection(dir);

	cout << "\nscan direction "<<dir << endl;
	aline.val = 0;
	for(int i=0;i<(SEARCH_DISTANCE*2-1);i++) {
		aline.val = aline.val << 1;
		aline.cnt++;
		curr_cell->print();
		cout << ",";
		if(curr_cell->val == val) {
			aline.val = aline.val | 0x1;
		}
	//	printf(",%x,",aline.val);

		curr_cell = curr_cell->near_ptr[dir];
		if(curr_cell->val&oppval)
			break;
	}
	cout << endl;
	return aline;
}

void Line::print(){
	printf("Line=0x%X Cnt=%d Score=%d",val,cnt,score);
	cout << endl;
}
