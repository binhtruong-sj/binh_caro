/*f
 * caro.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//#define VERBOSE3 `1

#ifdef VERBOSE3
#define VERBOSE2 1
#endif

#ifdef VERBOSE2
#define VERBOSE 1
#endif

//#define VERBOSE2 1
#define VERBOSE0 1

#define HASH 1

#include "caro.h"
hashTable ahash;
void cell::print(int mode){
	 if(mode==0) {
		 printf("%2X",val);
	 } else if (mode ==SYMBOLMODE){
		char ach=	val==0x1 ? 'X':
					val==0x2 ? 'O':
					val==E_FAR ? '-':
					val==0xb ? 'b':
					val==E_NEAR ? '\"': '.';
		printf("%2c ",ach);
	 } else {
		 printf("%2x ", score);
	 }
}

void cell::print(){
		 cout << val;
}

caro::caro(int table_size) {
	size = table_size+1;
// Setup pointer to ajacent cells, only from 1 to size-1
	aScoreArray.reserve(size*size);
	for(int row=0; row<=size; row++){
		for(int col=0; col<=size; col++){
			board[row][col].rowVal = row;
			board[row][col].colVal = col;
			if((row==0)||(col==0)||(row==size)||(col==size)) {
				board[row][col].val = BOUNDARY;
				for(int i=0; i<8; i++)
					board[row][col].near_ptr[i] = &board[row][col];
			}else{
				board[row][col].val = E_FAR; // FAR: empty cell 5 away from any occupied cell
				board[row][col].near_ptr[West] = &board[row][col-1];
				board[row][col].near_ptr[NWest] = &board[row-1][col-1];
				board[row][col].near_ptr[North] = &board[row-1][col];
				board[row][col].near_ptr[NEast] = &board[row-1][col+1];

				board[row][col].near_ptr[East] = &board[row][col+1];
				board[row][col].near_ptr[SEast] = &board[row+1][col+1];
				board[row][col].near_ptr[South] = &board[row+1][col];
				board[row][col].near_ptr[SWest] = &board[row+1][col-1];
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
void caro::print(int mode) {
    cout << endl;
	for(int row=0; row<=size; row++){
		for(int col=0; col<=size; col++){
			board[row][col].print(mode);
		}
		cout << " ROW " << row << endl;
	}
	cout <<"   ";
	for(char pchar = 'A'; pchar <='P'; pchar++)
			printf("%2C ",pchar);
	cout << endl;
}

void caro::print() {
    cout << endl;
	for(int row=0; row<=size; row++){
		for(int col=0; col<=size; col++){
			board[row][col].print(0);
		}
		cout << " ROW " << row << endl;
	}
	for(char pchar = 'A'; pchar <='P'; pchar++)
			printf("%2C ",pchar);
	cout << endl;
}
void caro::clearScore(){
	for(int row=0; row<=size; row++)
			for(int col=0; col<=size; col++)
				board[row][col].score = 0;
}
/*
 * set a cell to X or O
 */
int caro::setCell(int setVal, int row, int col, int near) {
	if((board[row][col].val &0x3) == 0) {
		board[row][col].val = setVal;
		if(near==E_NEAR)
			setNEAR(row,col);
		else
			setTNEAR(row,col);

		return 0;
	}
	else
		return 1;
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */
void caro::setNEAR(int row,int col) {
	cell *currCell;
	for(int dir=East; dir<=SEast; dir++) {
		currCell = &board[row][col];
		for(int i = 0;i <InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			if((currCell->val & 0x3)==0) {
				currCell->val = E_NEAR;
			}else if((currCell->val & 0x3)==0x3)
				break;
		}
	}
}

/*
 * Temporary near marking set when traverse further ahead
 */
void caro::setTNEAR(int row,int col) {
	cell *currCell;
	for(int dir =East; dir<=SEast; dir++) {
		currCell = &board[row][col];
		for(int i = 0;i <InspectDistance; i++) {
			currCell = currCell->near_ptr[dir];
			if((currCell->val & 0x3)==0x3)
				break;
			else if((currCell->val & 0x7)==0)
				currCell->val = E_TNEAR;
		}
	}
}
/*
 * After temporary setting a Cell to X' or O' for scoring, now return it and its neighbor to prev values
 */
void caro::restoreCell(int saveVal, int row,int col) {
	cell *currCell;
	board[row][col].val = saveVal; // Return the val to prev

	for(int dir=East; dir<=SEast; dir++) {
		currCell = &board[row][col];
		for(int i = 0;i <4; i++) {
			currCell = currCell->near_ptr[dir];
			if(currCell->val==E_TNEAR) currCell->val = E_FAR;  // only clear the cell with E_TNEAR (temporary NEAR) to FAR
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
	for(int i=0;i<SEARCH_DISTANCE;i++) {
#ifdef VERBOSE3
		currCell->print(SYMBOLMODE);
#endif
		if(currCell->val&oppval) {
			found_opp = 1;
			break;
		}
		backoffCell = currCell;
		currCell = currCell->near_ptr[dir];
	}
	if(found_opp) {
		//currCell = currCell->near_ptr[ReverseDirection(dir)]; // backoff 1
		currCell = backoffCell;

	} else {
		// Did not find O_, switch back to ori ad scan in reverse direction
		currCell = oriCell;
		dir = ReverseDirection(dir);
#ifdef VERBOSE3
		cout << "\nscan direction "<<dir << endl;
#endif

		for(int i=0;i<SEARCH_DISTANCE;i++) {
#ifdef VERBOSE3
			currCell->print(SYMBOLMODE);
#endif
			if(currCell->val&oppval) {
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
#ifdef VERBOSE3
	cout << "\nFinal scan direction "<<dir << endl;
#endif
	aline.val = 0;
	for(int i=0;i<(SEARCH_DISTANCE*2-1);i++) {
		aline.val = aline.val << 1;
		aline.cnt++;
#ifdef VERBOSE3
		currCell->print(SYMBOLMODE);
#endif
		if(currCell->val == val) {
			aline.val = aline.val | 0x1;
		}
		currCell = currCell->near_ptr[dir];
		if(currCell->val&oppval)
			break;
	}

	return aline;
}


void Line::print(){
	printf("Line=0x%X Cnt=%d Score=%d",val,cnt,score);
	cout << endl;
}
int Line::evaluate(){
	// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
	// self learning)
	score =0;
	int tval = val;
	int bitcnt = 0;
	while(tval){
		bitcnt = bitcnt+ (tval & 1);
		tval = tval >> 1;
	}
	score = (bitcnt*bitcnt*bitcnt*bitcnt*5)/cnt;

#ifdef VERBOSE9
	int a;
	cin >> a ;
	printf("Line=%x, cnt=%d, bitcnt=%d, Score=%d\n",val,cnt,bitcnt,score);
#endif
#ifdef HASH
		ahash.addEntry(val,cnt,score);
#endif
	return score;
}

void FourLines::print(){
	for(int dir=East; dir<West; dir++){
		Xlines[dir].print();
	}
}


int caro::score1Cell(int setVal, int row, int col){
	FourLines astar;
	astar.score = 0;
	int saveVal = board[row][col].val;
	setCell(setVal,row,col,E_TNEAR);
		for(int dir=East; dir<West; dir++){
			astar.Xlines[dir] = extractLine(dir,row,col);
			astar.score = astar.score + astar.Xlines[dir].evaluate();
		}
	board[row][col].score += astar.score;
	restoreCell(saveVal,row,col);

	return astar.score;
}
bool morecmd(scoreElement &s1, scoreElement &s2) {
	return(s1.val > s2.val);
}
scoreElement caro::evalAllCell(int setVal, int width, int depth){

	cell *cPtr;
	scoreElement bestScore;
	scoreElement *myScorePtr;
	int opnVal = oppositeVal(setVal);
	myScorePtr = new scoreElement[width];

	aScoreArray.clear();
	clearScore();

	for(int row=1; row<size; row++) {
		for(int col=1; col<size; col++ ) {
			if(board[row][col].val & (E_NEAR | E_TNEAR)) {
				bestScore.val = score1Cell(setVal,row,col) ; // need to have negative bias for opponent
				bestScore.val = bestScore.val + score1Cell(opnVal,row,col);
				bestScore.cellPtr = &board[row][col];
				aScoreArray.push_back(bestScore);
			}
		}
	}
	sort(aScoreArray.begin(), aScoreArray.end(),morecmd);
#ifdef VERBOSE00
	char astring[20];
	print(SCOREMODE);
	cout << "Top Score " << endl;
	cin >> astring;
#endif
	for(int i=0; i<width; i++){
			myScorePtr[i] =aScoreArray[i];


#ifdef VERBOSE0
	if(depth==5){
		cPtr = myScorePtr[i].cellPtr;
		printf("Score=0x%x,(%d,%c)d=%d \n",myScorePtr[i].val,cPtr->rowVal,cPtr->colVal-1+'A',depth);
	}
#endif
	}
	bestScore = myScorePtr[0]; // BestScore

	if(depth>0) {
		for(int i=0; i <width; i++) {
			cPtr = myScorePtr[i].cellPtr;
			int saveVal =cPtr->val;
			setCell(setVal,cPtr->rowVal,cPtr->colVal,E_TNEAR);
		///////
			myScorePtr[i] = evalAllCell(opnVal,width,depth-1);
#ifdef VERBOSE0
			if(depth == 5) {
			printf("d=%d, i=%d, Nscore =%x, pBest=%x",depth,i,myScorePtr[i].val, bestScore.val);
			cout << endl;
			}
#endif
			if(myScorePtr[i].val > bestScore.val) {
				bestScore.val = myScorePtr[i].val;
				bestScore.cellPtr = cPtr;
			}
			restoreCell(saveVal,cPtr->rowVal,cPtr->colVal);

		}
	}
	delete(myScorePtr);
	return bestScore;
}

hashTable::hashTable(){
	arrayE_cnt =0;
}

void hashTable::addEntry(int line, int bitcnt, int score){
	int i;
	for( i=0;i<arrayE_cnt;i++){
		if((arrayE[i].line == line)&&(arrayE[i].bitcnt==bitcnt)){
			arrayE[i].refcnt++;
			return;
		}
	}
	if(i > 1000) {
		cout << "hashE cnt too small" << endl;
	}{
		arrayE[arrayE_cnt].line = line;
		arrayE[arrayE_cnt].bitcnt = bitcnt;
		arrayE[arrayE_cnt++].score = score;
	}
}

void hashTable::print(){
	cout << "Hash Table: count = " << arrayE_cnt << endl;
	printf("%4s %8s %8s %8s %8s %8s\n","no", "Line","bitcnt", "score", "RefCnt", "Binary");
	for(int i;i<arrayE_cnt;i++){
		char binary[9];
		int val = arrayE[i].line;
		for(int bit=0; bit<8; bit++) {
			binary[bit] = val & 0x1;
			val >>= 1;
		}
		binary[8]= 0;
		printf("%4d %8x %8d %8d %8d %8s\n",i,arrayE[i].line,arrayE[i].bitcnt,arrayE[i].score,arrayE[i].refcnt,binary);
	}
}
