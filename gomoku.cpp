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
#define isX(a) a=='X'?X_:O_
#define isNotX(a) a=='X'?O_:X_
#define INF 999999999
#define NINF -INF
/*
 * For the purpose of testing the code.  This is a textfile setting the game up
 * after setting,
 * mode=1 for continue testing
 * mode=0 will prompt for manual typing
 * testing format: Direction row col (direction>7 terminate the test)
 *
 */
void help() {
	cout << "-3 debugScoring " << endl
			<< "-4 debugBestPath specify path for debug, and at which depth to "
			<< endl << "display detail of line scoring " << endl
			<< "-11 debug line scoring" << endl
			<< "-23 debugScoringd, like best path, but for all instead" << endl
			<< "-24 display score for every move" << endl;

}
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
	agame->clearScore();
	agame->print(SYMBOLMODE);
}

int main() {
	caro agame(15);
	int row, col, mode, dir;
	char name[4], testType[20];
	FILE *finput;
	Line tempLine;
	points width = 5;
	int depth = 5;
	extern int search_depth, search_width;
	extern tsDebug aDebug;
	extern int debugScoring, debugScoringd, debugScoringAll, debugBestPath,
			debugAllPaths, debugAI, debugAIbest, debugHash;
	extern int interactiveDebug;
	int twoPass = 0;
	bool redonext = false;

	bool maximizingPlayer = true;
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
	cout << fn << endl;
	finput = fopen(fn.c_str(), "r");
	for (int i = search_depth; i >= 0; i--) {
		aDebug.debugWidthAtDepth[i] = -9999;
	}
	aDebug.debugWidthAtDepth[search_depth] = 0;
	if (finput) {
		bool ending = false;

		getInput(finput, &agame);
		agame.clearScore();

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
				do {
					cout << "enter X dir row col" << endl;
					scanf("%s %d %d %c", testType, &dir, &row, name);
					col = convertToRow(name[0]);
					cout << row << " " << col << "name=" << name << testType
							<< endl;
					agame.setCell(isX(testType[0]), row, col, E_NEAR);
					agame.print(SYMBOLMODE);

					tempLine = agame.extractLine(X_, dir, row, col, ending,
							true);
					tempLine.print();
					agame.restoreCell(0, row, col);

				} while (dir < 10);
				break;
			case '4': {
				FourLines astar;
				for (int dir = 0; dir < 4; dir++) {
					astar.Xlines[dir] = agame.extractLine(X_, dir, row, col,
							ending, true);
				}
				astar.print();
				break;
			}
			case 's': {
				Line tempLine = agame.extractLine(X_, dir, row, col, ending,
						true);
				tempLine.evaluate(ending);
				tempLine.print();
			}
				break;

			case 'X': // score 1 cell
				debugScoringAll = 1;
				cout << "Score ";
				cout << agame.score1Cell(X_, row, col, 0, true);
				cout << " for X at row " << row;
				cout << "col " << convertToChar(col) << endl;
				agame.print(SCOREMODE);
				break;
			case 'O': // score 1 cell
				debugScoringAll = 1;

				cout << "Score " << agame.score1Cell(O_, row, col, 0, true)
						<< " for O at row " << row << "col "
						<< convertToChar(col) << endl;
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
					result = agame.evalAllCell(X_, width, depth, 0,
							!maximizingPlayer, NINF, INF, top_bc, false,
							redonext, nullptr); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_, width, depth, 0,
							!maximizingPlayer, NINF, INF, top_bc, false,
							redonext, nullptr);
				printf("Score=%x, row=%d, col=%C", result.val,
						(result.cellPtr)->rowVal,
						(result.cellPtr)->colVal - 1 + 'A');
				agame.print(SYMBOLMODE);
				agame.print(SCOREMODE);
				agame.print(SCOREMODEm);

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
				aDebug.debugBreakAtDepth = -12345;
				while (col < 20) {
					int redo = 1;
					do {

						cout << "Enter row col X/O" << endl;
						cout
								<< "-1 undo -2 redo -3 debugScoring, -4 debugBestPath,"
								<< "-5 debugBestPath -6 debugAllPaths, -7 debugAI, "
								<< "-10 TRACE-debug -11 cells , "
								<< "-12 reset debug, -13 save -23 debugScoringd -24 debugScoringAll"
								<< " -25 debugHash -99 interactiveDebug "
								<< " enter ? on col for details" << endl;

						cin >> row;
						if (row <= -1) {
							agame.cdbg.reset();

							switch (row) {
							case -10: {
								char adebugstr[40];
								cout << "Enter TRACE debug:";
								cin >> adebugstr;
								agame.cdebug.enterDebugTrace(adebugstr);
								cout << "Enter Cellinfo:";
								cin >> adebugstr;
								agame.cdebug.enterDebugCell(adebugstr);
								break;
							}
							case -11: {
								int row, col;
								char ccol[10];
								int depth, debugLine;
								agame.cdbg.reset();

								do {
									cout << "Enter row:  (-1 to quit)";
									cin >> row;
									if (row < 0)
										break;
									cout << "Enter col: ";
									cin >> ccol;
									col = ccol[0] - 'a' + 1;
									cout << "Enter Depth: ";
									cin >> depth;
									cout << "Enter debugLine: ";
									cin >> debugLine;
									agame.cdbg.add(&agame.board[row][col],
											depth, debugLine);

								} while (row > 0);
								break;
							}
							case -12:
								for (int i = search_depth; i >= 0; i--) {
									aDebug.debugWidthAtDepth[i] = -1;
								}
								aDebug.debugWidthAtDepth[search_depth] = 0;
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
							case -23:
								cout << "Turn " << FLIP(debugScoringd)
										<< " debugScoringd" << endl;

								break;
							case -24:
								cout << "Turn " << FLIP(debugScoringAll)
										<< " debugScoringAll" << endl;

								break;
							case -25:
								cout << "Turn " << FLIP(debugHash)
										<< " debugHash" << endl;

								break;

							case -3:
								cout << "Turn " << FLIP(debugScoring)
										<< " debugScoring" << endl;

								break;
							case -99:
								cout << "Turn " << FLIP(interactiveDebug)
										<< " interactiveDebug" << endl;

								break;

							case -4:
								agame.cdbg.reset();

								cout << "Turn " << FLIP(debugBestPath)
										<< " debugBestPath" << endl;
								if (debugBestPath) {
									cout
											<< "Enter debug width ID at each depth ";
									for (int i = depth; i >= 0; i--) {
										int enterWidth;
										cout << "at Depth= " << i << " = ";
										cin >> enterWidth;
										aDebug.debugWidthAtDepth[i] =
												enterWidth;
									}
									cout
											<< "Enter depth # to break for prompt: ";
									cin >> aDebug.debugBreakAtDepth;
									if (aDebug.debugBreakAtDepth == (depth))
										aDebug.enablePrinting = 1;
									else
										aDebug.enablePrinting = 0;
								}
								break;
							case -5:
								agame.cdbg.reset();

								cout << "Turn " << FLIP(debugBestPath)
										<< " debugBestPath" << endl;
								if (debugBestPath)
									twoPass = 1;
								else
									twoPass = 0;
								for (int i = depth; i >= 0; i--) {
									aDebug.debugWidthAtDepth[i] = -1;
								}
								aDebug.debugBreakAtDepth = -10;
								break;
							case -6:
								cout << "Turn " << FLIP(debugAllPaths)
										<< " debugAllPaths" << endl;
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
										cin >> aDebug.debugWidthAtDepth[i];
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
					char gameCh;
					cin >> name >> gameCh;
					if (name[0] == '?')
						help();

					col = name[0] - 'a' + 1;
					agame.aiPlay = O_;
					scoreElement result;
					agame.maxDepth = depth;
					cell * aptr = agame.setCell(isX(gameCh), row, col, E_NEAR);
					breadCrumb top_bc(depth); // at this level is depth+1
					top_bc.top.ptr = aptr;
					for (int passNo = 0; passNo <= twoPass; passNo++) {
						cout << "Pass no " << passNo << endl;
						agame.terminate = 0;
						if (twoPass & (passNo == 0)) {
							agame.cdbg.reset();
							aDebug.lowDepth = depth + 1;
							for (int d = depth; d >= 0; d--) {
								aDebug.debugWidthAtDepth[d] = -999;
							}
						}
						agame.evalCnt = agame.myMoveAccScore =
								agame.opnMoveAccScore = 0;
						agame.aiPlay = isNotX(gameCh);
						int tw =
								passNo > 0 ?
										aDebug.debugWidthAtDepth[depth] : 0;
						agame.trace.clear();
						{
							bool debugThis = false;

							if (interactiveDebug) {
								debugThis = true;
							}
							do {
								redonext = false;
								agame.scorecnt = agame.skipcnt = 0;
								result = agame.evalAllCell(isNotX(gameCh),
										width, depth, tw, maximizingPlayer,
										NINF, INF, top_bc, debugThis, redonext,
										nullptr);
							} while (redonext);

						}
						aDebug.enablePrinting = 0;

						cout << top_bc << endl;
						hist pArray;

						top_bc.extractTohistArray(pArray);
						cout << "_____________________________________________________________\n";
						agame.print(pArray);

						cout << "aDebug.debugWidthAtDepth " << endl;

						for (int d = depth - 1; d >= 0; d--) {
							aDebug.debugWidthAtDepth[d] =
									top_bc.bestWidthAtDepth(d + 1); // from the higher best
							agame.cdbg.add(top_bc.bestCellAtDepth(d + 1), d, 0);
							cout << "  d=" << d << "w="
									<< aDebug.debugWidthAtDepth[d];

						}
						//		agame.cdbg.add(top_bc.bestCellAtDepth(0), 0, 0);
						cout << endl;
					}
					char ans;
					cout
							<< "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
					cout << "\nRESULT= " << *result.cellPtr << endl;
					cout << "set it ?";
					cin >> ans;

					cout
							<< "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
					if (ans == 'y') {
						agame.setCell((int) (isNotX(gameCh)),
								result.cellPtr->rowVal, result.cellPtr->colVal,
								E_NEAR);

					} else {
						cout << "Enter row col val" << endl;
						int row;
						char ach, ccol;
						//	agame.print(SYMBOLMODE);
						agame.print(SCOREMODE);

						cin >> row >> ccol >> ach;
						int col = ccol - 'a' + 1;
						cout << row << " " << ccol << " " << (int) (isX(ach))
								<< endl;
						agame.setCell((int) (isX(ach)), row, col, E_NEAR);
					}

					ahash.print();
					if (twoPass) {
						cout << "lowestD=" << aDebug.lowDepth << endl;
						cout << top_bc << endl;
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
