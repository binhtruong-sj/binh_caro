//============================================================================
// Name        : gomoku.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <locale>
using namespace std;
#include "caro.h"
#define convertToRow(a) isalpha(a)? (islower(a)? (a-'a'+1):(a-'A'+1)):1

/*
 * For the purpose of testing the code.  This is a textfile setting the game up
 * after setting,
 * mode=1 for continue testing
 * mode=0 will prompt for manual typing
 * testing format: Direction row col (direction>7 terminate the test)
 *
 */
void getInput(FILE *finput,caro *agame) {
	char aline[20], *cptr;
	int row,col;
	row = 1;
	do{
		fgets(aline,80,finput);
		cptr = aline;
		col = 0;
		while(char achar = *cptr++){
			if(achar == 'M') {
				row=0;col = 0;
				break;
			} else if(achar == 'L') {
				row=-100; // quit
				break;
			} else if(achar == 'b') {
				if(col >1) {
					break;
				} else {
					col =0;
				}
			} else if(achar == 'X') {
			  agame->setCell(X_,row,col,E_NEAR);
			} else if (achar == 'O') {
			  agame->setCell(O_,row,col,E_NEAR);
			}
			col++;
		}
	} while(++row>0);
	fgets(aline,80,finput);
	cout << endl;
	agame->print(1);
}

int main() {
	caro agame(15);
	int row,col,mode,dir;
	char name[4],testType[20];
	FILE *finput;
	Line tempLine;
extern	hashTable ahash;

	//agame.print();
#if 1
	finput = fopen("testinput.txt","r");
	if(finput) {
		getInput(finput,&agame);
		fscanf(finput,"%d",&mode);
		dir =0;
		while(dir <8) {
			if(mode)
				fscanf(finput,"%s %d %d %c",testType,&dir,&row,name);
			else
				scanf("%s %d %d %c",testType,&dir,&row,name);
			cout << "------------------" << testType << " " << dir << " " << row << " " << name << endl;
			col = convertToRow(name[0]);
			if(testType[0] =='q') {
				break;
			}else if(testType[0] =='e') {
				tempLine = agame.extractLine(dir,row,col);
				tempLine.print();
			}else if(testType[0] =='4') {
				FourLines astar;
				for(int dir = 0;dir <4; dir++){
					astar.Xlines[dir] = agame.extractLine(dir,row,col);
				}
				astar.print();
			}else if(testType[0] =='s') { // score 1 line
				Line tempLine = agame.extractLine(dir,row,col);
				tempLine.evaluate();
				tempLine.print();

			}else if(testType[0] =='X') { // score 1 cell
				cout << "Score " << agame.score1Cell(X_,row,col) <<  " for X at row " << row << "col " << col << endl;
				agame.print(SCOREMODE);
			}else if(testType[0] =='O') { // score 1 cell
				cout << "Score " << agame.score1Cell(O_,row,col) <<  " for O at row " << row << "col " << col << endl;
				agame.print(SCOREMODE);

			}else if(testType[0] =='e') {
							tempLine = agame.extractLine(dir,row,col);
							tempLine.print();
			}else if(testType[0] =='A') {
				agame.print(SYMBOLMODE);
				int width = dir;
				int depth = row;
				scoreElement result;
				if(testType[1] =='X')
					result = agame.evalAllCell(X_,width,depth); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_,width,depth);
				printf("Score=%x, row=%d, col=%C",result.val,(result.cellPtr)->rowVal,(result.cellPtr)->colVal-1+'A');
				agame.print(SYMBOLMODE);
				agame.print(SCOREMODE);
			}else if(testType[0] =='g') {
				mode = 0;
				caro newgame(15);
				col = 1;
				int width = 5;
				int depth = 5;
				while(col < 20){
					cout << "Enter row col: " << endl;
					scanf("%d %c",&row,name);
					col = name[0]-'a'+1;
					newgame.setCell(X_,row,col,E_NEAR);
					newgame.print(SYMBOLMODE);
					scoreElement result = newgame.evalAllCell(O_,width,depth);
					printf("Score=%x, row=%d, col=%C",result.val,(result.cellPtr)->rowVal,(result.cellPtr)->colVal-1+'A');
					cout << endl;
					newgame.setCell(O_,(result.cellPtr)->rowVal,(result.cellPtr)->colVal,E_NEAR);
					newgame.print(SYMBOLMODE);
					ahash.print();
				}
				break;
			}else if(testType[0] =='G') {
					mode = 0;
					col = 1;
					int width = 5;
					int depth = 5;
					while(col < 20){
						cout << "Enter row col: " << endl;
						scanf("%d %c",&row,name);
						col = name[0]-'a'+1;
						agame.setCell(X_,row,col,E_NEAR);
						agame.print(SYMBOLMODE);
						scoreElement result = agame.evalAllCell(O_,width,depth);
						printf("Score=%x, row=%d, col=%C",result.val,(result.cellPtr)->rowVal,(result.cellPtr)->colVal-1+'A');
						cout << endl;
						agame.setCell(O_,(result.cellPtr)->rowVal,(result.cellPtr)->colVal,E_NEAR);
						ahash.print();						ahash.print();
						agame.print(SYMBOLMODE);

					}
				break;
			} else{
				break;
			}
		}
	}
#endif
	cout << endl << "Life is good!" << endl;
	return 0;
}
