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
void getInput(FILE *finput, caro *agame) {
	char aline[20], *cptr;
	int row, col;
	row = 1;
	do {
		fgets(aline, 80, finput);
		cptr = aline;
		col = 0;
		while (char achar = *cptr++) {
			if (achar == 'M') {
				row = 0;
				col = 0;
				break;
			} else if (achar == 'L') {
				row = -100; // quit
				break;
			} else if (achar == 'b') {
				if (col > 1) {
					break;
				} else {
					col = 0;
				}
			} else if (achar == 'X') {
				agame->setCell(X_, row, col, E_NEAR);
			} else if (achar == 'O') {
				agame->setCell(O_, row, col, E_NEAR);
			}
			col++;
		}
	} while (++row > 0);
	fgets(aline, 80, finput);
	cout << endl;
	agame->print(1);
}

int main() {
	caro agame(15);
	int row, col, mode, dir;
	char name[4], testType[20];
	FILE *finput;
	Line tempLine;
	int width = 5;
	int depth = 5;
	extern int search_depth, search_width, debugWidthAtDepth[20];
	extern tsDebug aDebug;
	extern int debugScoring, debugScoringE, debugHash, debugAI, debugAIbest;
	extern int debugRow, debugCol;
	extern scoreElement bestPath[];
	extern hashTable ahash;
	//agame.print();
#if 1
	cout << "Enter filename " << endl;
	string aname;
	cin >> aname;
	string fn = "testinput.txt";
	fn = fn + aname;
	finput = fopen(fn.c_str(), "r");
	if (finput) {
		getInput(finput, &agame);
		fscanf(finput, "%d", &mode);
		dir = 0;
		while (dir < 8) {
			if (mode)
				fscanf(finput, "%s %d %d %c", testType, &dir, &row, name);
			else
				scanf("%s %d %d %c", testType, &dir, &row, name);
			cout << "------------------" << testType << " " << dir << " " << row
					<< " " << name << endl;
			col = convertToRow(name[0]);
			switch (testType[0]) {
			case 'q':
				break;
			case 'e':
				tempLine = agame.extractLine(dir, row, col);
				tempLine.print();
				break;
			case '4': {
				FourLines astar;
				for (int dir = 0; dir < 4; dir++) {
					astar.Xlines[dir] = agame.extractLine(dir, row, col);
				}
				astar.print();
				break;
			}
			case 's': {
				Line tempLine = agame.extractLine(dir, row, col);
				tempLine.evaluate();
				tempLine.print();
			}
				break;

			case 'X': // score 1 cell
				cout << "Score " << agame.score1Cell(X_, row, col)
						<< " for X at row " << row << "col " << col << endl;
				agame.print(SCOREMODE);
				break;
			case 'O': // score 1 cell
				cout << "Score " << agame.score1Cell(O_, row, col)
						<< " for O at row " << row << "col " << col << endl;
				agame.print(SCOREMODE);
				break;
				;
			case 'A': {
				agame.print(SYMBOLMODE);
				int width = dir;
				int depth = row;
				scoreElement result;
				if (testType[1] == 'X')
					result = agame.evalAllCell(X_, width, depth, 0); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_, width, depth, 0);
				printf("Score=%x, row=%d, col=%C", result.val,
						(result.cellPtr)->rowVal,
						(result.cellPtr)->colVal - 1 + 'A');
				agame.print(SYMBOLMODE);
				agame.print(SCOREMODE);
			}
				break;
			case 'G':
				mode = 0;
				col = 1;
				cout << "NEW GAME" << endl;
				cout << "Enter width, depth : " << endl;
				;
				cin >> search_width >> search_depth;
				agame.reset();
				width = search_width;
				depth = search_depth;

			case 'g':
				while (col < 20) {
					int redo = 1;
					do {
						cout << "Enter row col " << endl;
						cout
								<< "-1 undo -2 redo -3 debugScoring, -4 debugScoringE,"
								<< "-5 debugHash, -6 debugAI " << endl;
						cin >> row;
						if (row <= -1) {
							switch (row) {
							case -3:
								cout << "Turn " << FLIP(debugScoring)
										<< " debugScoring" << endl;
								if (debugScoring) {
									cout << "Enter Debug Row Col:";
									cin >> debugRow >> name;
									debugCol = name[0] - 'a' + 1;
								}
								break;

							case -4:
								cout << "Turn " << FLIP(debugScoringE)
										<< " debugScoringE" << endl;
							case -5:
								cout << "Turn " << FLIP(debugHash)
										<< " debugHash" << endl;
								if (row == -5) {
									cout << "Turn " << FLIP(debugAIbest)
											<< " debugAIbest" << endl;
								}
								break;
							case -6:
								cout << "Turn " << FLIP(debugAI) << " debugAI"
										<< endl;
								if (debugAI) {
									cout
											<< "Enter the % of I at each Depth Search: "
											<< endl;
									for (int i = search_depth; i >= 0; i--) {
										cin >> debugWidthAtDepth[i];
									}
								}
								break;
							case -1:
								agame.undo1move();
								agame.print(SYMBOLMODE);
								break;

							case -2:
								agame.redo1move();
								agame.print(SYMBOLMODE);
								break;

							default:
								break;
							}
						} else
							redo = 0;
					} while (redo);
					cin >> name;
					col = name[0] - 'a' + 1;
					agame.myVal = O_;
					agame.setCell(X_, row, col, E_NEAR);
					agame.terminate = 0;
					scoreElement result = agame.evalAllCell(O_, width, depth,
							0);
					cout
							<< "-------------------------------------------------------------------------------------"
							<< endl;

					printf("Score=%x, row=%d, col=%C", result.val,
							(result.cellPtr)->rowVal,
							(result.cellPtr)->colVal - 1 + 'A');
					cout << endl;
					agame.setCell(O_, (result.cellPtr)->rowVal,
							(result.cellPtr)->colVal, E_NEAR);
					ahash.print();
					aDebug.print(depth, width);
					for (int i = depth; i >= 0; i--) {
						printf("Depth=%d bestScore=%x, row=%d, col=%c\n", i,
								bestPath[i].val, bestPath[i].cellPtr->rowVal,
								bestPath[i].cellPtr->colVal - 1 + 'a');

					}
					agame.print(SYMBOLMODE);
				}
				agame.reset();
				break;
			}
		}
	}
#endif
	cout << endl << "Life is good!" << endl;
	return 0;
}
