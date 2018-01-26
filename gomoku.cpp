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
#define FLIP(a) ((a=a^1)? "ON":"OFF")
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
	int width=5;
	int depth=5;
	extern int search_depth, search_width, debugWidthAtDepth[20];
	extern tsDebug aDebug;
	extern int debugScoring,debugScoringE, debugHash, debugAI;
	extern int debugRow, debugCol;
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
					result = agame.evalAllCell(X_,width,depth,0); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_,width,depth,0);
				printf("Score=%x, row=%d, col=%C",result.val,(result.cellPtr)->rowVal,(result.cellPtr)->colVal-1+'A');
				agame.print(SYMBOLMODE);
				agame.print(SCOREMODE);
			}else if(testType[0] =='G') {
				mode = 0;
				col = 1;
				cout << "Enter width, depth : " << endl;;
				cin >> search_width >> search_depth;
				agame.reset();
				width = search_width;
				depth = search_depth;

			}else if(testType[0] =='g') {
				while(col < 20){
					int redo =1;
					do {
						cout << "Enter row (-1 for undo, -2 for debug option) col: " << endl;
					//	scanf("%d %c",&row,name);
						cin >> row;
						if(row < -1) {
							printf("-2 debugScoring, -3 debugScoringE, -4 debugHash, -5 debugAI ");
							cout << endl;
							if(row == -2) {
								cout << "Turn " << FLIP(debugScoring) << " debugScoring" << endl;
								if(debugScoring){
									cout << "Enter Debug Row Col:" ;
									cin >> debugRow >> name;
									debugCol = name[0]-'a'+1;
								}
								}
							if(row == -3)
								cout << "Turn " << FLIP(debugScoringE) << " debugScoringE" << endl;
							if(row == -4)
								cout << "Turn " << FLIP(debugHash) << " debugHash" << endl;
							if(row == -5){
								cout << "Turn " << FLIP(debugAI) << " debugAI" << endl;
								if(debugAI) {
										cout << "Enter the % of I at each Depth Search: " << endl;
										for(int i=search_depth;i>=0;i--){
											cin >> debugWidthAtDepth[i];
										}
								}
							}
						} else if(row == -1) {
							agame.undo1move();
							agame.print(SYMBOLMODE);
						} else
							redo = 0;
					}while(redo);
					cin >> name;
					col = name[0]-'a'+1;
					agame.myVal = O_;
					agame.setCell(X_,row,col,E_NEAR);
					agame.terminate = 0;
					scoreElement result = agame.evalAllCell(O_,width,depth,0);
					cout <<"-------------------------------------------------------------------------------------" << endl;

					printf("Score=%x, row=%d, col=%C",result.val,(result.cellPtr)->rowVal,(result.cellPtr)->colVal-1+'A');
					cout << endl;
					agame.setCell(O_,(result.cellPtr)->rowVal,(result.cellPtr)->colVal,E_NEAR);
					ahash.print();
					aDebug.print(depth,width);
					agame.print(SYMBOLMODE);
				}
				agame.reset();
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
