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
//		cout << "Line:" << aline << endl;
		col = 0;
		while(char achar = *cptr++){
//			cout << "char: " << achar << endl;
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
			  agame->setcell(X_,row,col);
//				agame->print();

			} else if (achar == 'O') {
			  agame->setcell(O_,row,col);
//				agame->print();
			}
			col++;
		}
	} while(++row>0);
	fgets(aline,80,finput);
	cout << endl;
	agame->print();
}

int main() {
	caro agame(15);
	int row,col,mode,dir;
	char name[4];
	FILE *finput;
	Line tempLine;
	//agame.print();
#if 1
	finput = fopen("testinput.txt","r");
	if(finput) {
		getInput(finput,&agame);
		fscanf(finput,"%d",&mode);
		dir =0;
		while(dir <8) {
			if(mode)
				fscanf(finput,"%d %d %c",&dir,&row,name);
			else
				scanf("%d %d %c",&dir,&row,name);
			col = convertToRow(name[0]);
			if(dir <8) {
				tempLine = agame.extractLine(dir,row,col);
				tempLine.print();
			}
		}
	}
#endif
	cout << endl << "Goodbye" << endl;
	return 0;
}
