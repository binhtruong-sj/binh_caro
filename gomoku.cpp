//============================================================================
// Name        : gomoku.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <locale>
#include <string.h>
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
	char aline[80], *cptr;
	int row, col;
	row = 1000;
	do {
		fgets(aline, 80, finput);
		cptr = aline;
		col = 0;
		while (char achar = *cptr++) {
			if (achar != ' ') {
				if ((row > 30) && (achar == 'b')) {
					row = 0;
					col = 0;
					break;
				} else if ((achar == 'L') || (achar == 'A')) {
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
	unsigned int width = 5;
	int depth = 5;
	extern int search_depth, search_width, debugWidthAtDepth[];
	extern tsDebug aDebug;
	extern int debugScoring, debugScoringE, debugHash, debugAI, debugAIbest;
	int twoPass = 0;
	extern hashTable ahash;
	//agame.print();
#if 1
	cout << "Enter Width Depth " << endl;
	cin >> search_width >> search_depth;
	width = search_width;
	depth = search_depth;
	cout << "Enter filename " << endl;
	string aname;
	cin >> aname;
	string fn = "testinput.txt";
	fn = fn + aname;
	finput = fopen(fn.c_str(), "r");
	for (int i = search_depth; i >= 0; i--) {
		debugWidthAtDepth[i] = -9999;
	}
	debugWidthAtDepth[search_depth] = 0;
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
				breadCrumb top_bc(depth);
				if (testType[1] == 'X')
					result = agame.evalAllCell(X_, width, depth, 0, top_bc); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_, width, depth, 0, top_bc);
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
								<< "-5 debugBestPath -6 debugHash, -7 debugAI, -11 debugwidth , -12 reset debug, -13 save "
								<< endl;

						cin >> row;
						if (row <= -1) {
							switch (row) {
							case -11: {
								int debug_d, debug_w;
								cout << "Enter Depth";
								cin >> debug_d;
								cout << "Enter Width" << endl;
								cin >> debug_w;
								debugWidthAtDepth[debug_d] = debug_w;
								for (int i = search_depth; i >= 0; i--) {
									printf("D=%d,W=%d ", i,
											debugWidthAtDepth[i]);
								}
								cout << endl;
								break;
							}
							case -12:
								for (int i = search_depth; i >= 0; i--) {
									debugWidthAtDepth[i] = -1;
								}
								debugWidthAtDepth[search_depth] = 0;
								break;
							case -13: {
								char ffn[80] = "savefile.txt";
								for (int i = 1; i < 1000; i++) {
									sprintf(ffn, "savefile%d.txt", i);

									cout << "filename =" << ffn;
									ifstream ifile(ffn);
									if (ifile) {
										ifile.close();
										continue;
									}
									ofstream ofile(ffn);
									if (ofile) {
										char ans[80];
										cout << " Writing to " << ffn << " ?"
												<< endl;
										cin >> ans;
										if ((ans[0] == 'n')
												|| (ans[0] == 'N')) {
											ofile.close();
											continue;
										}
										ofile << agame;
										ofile.close();
										break;
									}

								}
							}
								break;
							case -3:
								cout << "Turn " << FLIP(debugScoring)
										<< " debugScoring" << endl;

								break;

							case -4:
								cout << "Turn " << FLIP(debugScoringE)
										<< " debugScoringE" << endl;
								cout << "Enter debug width ID at each depth ";
								for (int i = depth; i >= 0; i--) {
									int enterWidth;
									cout << "at Depth= " << i;
									cin >> enterWidth;
									debugWidthAtDepth[i] = enterWidth;
								}
								break;
							case -5:
								cout << "Turn " << FLIP(debugScoringE)
										<< " debugScoringBest" << endl;
								if (debugScoringE)
									twoPass = 1;
								else
									twoPass = 0;
								for (int i = depth; i >= 0; i--) {
									debugWidthAtDepth[i] = -1;
								}
								break;
							case -6:
								cout << "Turn " << FLIP(debugHash)
										<< " debugHash" << endl;
								if (row == -5) {
									cout << "Turn " << FLIP(debugAIbest)
											<< " debugAIbest" << endl;
								}
								break;
							case -7:
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
					scoreElement result;
					//			twoPass = 1;
					cell * aptr = agame.setCell(X_, row, col, E_NEAR);
					breadCrumb top_bc(depth); // at this level is depth+1
					top_bc.top.ptr = aptr;
					for (int passNo = 0; passNo <= twoPass; passNo++) {
						cout << "Pass no " << passNo << endl;
						agame.terminate = 0;
						if (twoPass & (passNo == 0)) {
							aDebug.lowDepth = depth + 1;
							for (int d = depth; d >= 0; d--) {
								debugWidthAtDepth[d] = -999;
							}
						}
						agame.evalCnt = agame.myMoveAccScore = agame.opnMoveAccScore = 0;
						result = agame.evalAllCell(O_, width, depth, 0, top_bc);
						agame.print(SYMBOLMODE);
						cout << top_bc << endl;
						cout << "bestWidthAtDepth ";
						debugWidthAtDepth[depth] = 0;
						for (int d = depth - 1; d >= 0; d--) {
							debugWidthAtDepth[d] = top_bc.bestWidthAtDepth(d);
							cout << "d=" << d << "w=" << debugWidthAtDepth[d];
						}
						cout << endl;
					}
					agame.setCell(O_, (result.cellPtr)->rowVal,
							(result.cellPtr)->colVal, E_NEAR);
					ahash.print();
					if (twoPass)
						cout << "lowestD=" << aDebug.lowDepth << endl;
					aDebug.print(depth, agame.widthAtDepth, debugWidthAtDepth);
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
