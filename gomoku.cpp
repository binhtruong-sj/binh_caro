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
#include <chrono>

using namespace std;
using namespace std::chrono;

#include "caro.h"
#define convertToRow(a) isalpha(a)? (islower(a)? (a-'a'+1):(a-'A'+1)):1
#define isX(a) a=='X'?X_:O_
#define isNotX(a) a=='X'?O_:X_
#define INF {0x0FFFFFFF,0,depth}
#define NINF {0,0x0FFFFFFF,depth}
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
	int setWidthDepth[][2] = { { 14, 8 }, { 14, 8 }, { 5, 10 }, { 4, 10 }, { 4,
			10 } };
	caro agame(15);
	int row, col, mode, dir;
	char name[4], testType[20];
	FILE *finput;
	Line tempLine;
	points width = 5;
	int depth = 5;
	extern int search_depth, search_width;
	extern int debugScoring, debugScoringd, debugScoringAll, debugHash, docheck;
	extern int interactiveDebug;
	bool redonext = false;

	bool maximizingPlayer = true;
	extern hashTable ahash;
	tracer aTracer;
	aTracer.prev = nullptr;
	aTracer.savePoint.cellPtr = &agame.board[1][1];
	aTracer.savePoint = {0,0,0};

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
				tempLine.evaluate(true, ending);
				tempLine.print();
			}
				break;

			case 'X': // score 1 cell
				debugScoringAll = 1;
				cout << "Score ";
				cout << agame.score1Cell(X_, row, col, true);
				cout << " for X at row " << row;
				cout << "col " << convertToChar(col) << endl;
				agame.print(SCOREMODE);
				break;
			case 'O': // score 1 cell
				debugScoringAll = 1;

				cout << "Score " << agame.score1Cell(O_, row, col, true)
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
					result = agame.evalAllCell(X_, width, depth,0,
							!maximizingPlayer, NINF, INF, false, redonext,
							nullptr, &aTracer); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_, width, depth,0,
							!maximizingPlayer, NINF, INF, false, redonext,
							nullptr, &aTracer);
				printf("Score=%x, row=%d, col=%C", result.myScore,
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
				cout << "Opponent is White==O,  Black==X ;";
				cout << "Enter X/O:";
				char gameCh;
				cin >> gameCh;
				if (gameCh == 'O') {
					agame.setCell(isNotX(gameCh), 8, 8, E_NEAR);
					agame.evalAllCell(isNotX(gameCh), 20, -1,0, maximizingPlayer,
					NINF, INF, false, redonext, nullptr, &aTracer);
				}
				while (col < 20) {
					int redo = 1;
					char fans[10], ccol;

					do {

						cout << "Enter row col" << endl;
						cout
								<< "-1 undo -2 redo -3 debugScoring, -4 debugBestPath,"
								<< "-5 debugBestPath -6 debugAllPaths, -7 debugAI, "
								<< "-10 TRACE-debug -11 cells , "
								<< "-12 reset debug, -13 save -23 debugScoringd -24 debugScoringAll"
								<< " -25 debugHash -98 docheck -99 interactiveDebug "
								<< " enter ? on col for details" << endl;

						cin >> fans;
						if (islower(fans[strlen(fans) - 1])) {
							sscanf(fans, "%d%c", &row, &ccol);
						} else {
							sscanf(fans, "%d", &row);
							ccol = '=';
						}

						if (row <= -1) {
							agame.cdbg.reset();

							switch (row) {

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
							case -98:
								cout << "Turn " << FLIP(docheck) << " docheck"
										<< endl;
							case -99:
								cout << "Turn " << FLIP(interactiveDebug)
										<< " interactiveDebug" << endl;
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
						int orow ;
						if (fans[0] == '?')
							help();
						if ((ccol == '=')) {
							cell* aptr = agame.possMove[row-1]; // -1 to not use zero
							col = aptr->colVal;
							orow = aptr->rowVal;
							row = mapping(orow);
							cout << *aptr;
							printf(" AT possMove  row=%d col=%d\n", row, col);
						} else {
							col = ccol - 'a' + 1;
						}
						if ((row > 15) || (col > 15)
								|| (agame.board[orow][col].val & (X_ | O_))) {
							cout << "Bad entry , val=" << hex
									<< agame.board[orow][col].val << endl;
							agame.print(SYMBOLMODE4);
							redo = 1;
						}
						cout << "fans=" << fans << " row=" << row << " col="
								<< col << endl;
					} while (redo);

					agame.my_AI_Play = O_;
					scoreElement result;
					agame.maxDepth = depth+10;
					int minDepth = 0;
					row = reverseMapping(row);
					agame.setCell(isX(gameCh), row, col, E_NEAR);
					agame.my_AI_Play = isNotX(gameCh);
					agame.trace.clear();
					bool debugThis = false;
					if (interactiveDebug) {
						debugThis = true;
					}
					agame.print(SYMBOLMODE);
					agame.rsum_chk_max = 0;
					agame.rsum_chk_min = 10;
					// Get starting timepoint
					auto start = high_resolution_clock::now();

					do {
						redonext = false;
						agame.localCnt = 0;
						result = agame.evalAllCell(isNotX(gameCh), width, depth+10,minDepth+10,
								maximizingPlayer,
								NINF, INF, debugThis, redonext, nullptr,
								&aTracer);
						if (redonext)
							delete aTracer.next;
					} while (redonext);
					delete aTracer.next;
					// Get ending timepoint
					auto stop = high_resolution_clock::now();

					// Get duration. Substart timepoints to
					// get durarion. To cast it to proper unit
					// use duration cast method
					auto duration = duration_cast<microseconds>(stop - start);
					float elapse = ((float) duration.count() / 1000000.0);
					cout << "Result =" << result << "  ";
					cout << "Time taken by function: " << elapse << " seconds"
							<< " at depth =" << depth << endl;

					int avg_chk = (agame.rsum_chk_max + agame.rsum_chk_min + 1)
							/ 2;
					width = setWidthDepth[avg_chk][0];
					int ndepth = setWidthDepth[avg_chk][1];
					if ((ndepth <= depth) && (elapse < 2.0)) {
						depth++;
					} else {
						depth = ndepth;
					}
					cout << "Avg chk = " << avg_chk << " depth=" << depth
							<< " Width=" << width << endl;
					char ans[10];

					/*
					 hist pArray;
					 << "_____________________________________________________________\n";
					 agame.print(pArray);

					 cout
					 << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
					 cout << "\nRESULT= " << *result.cellPtr << endl;
					 cout << "set it ? or (row col char)";
					 cin.clear();
					 cin.ignore(numeric_limits<streamsize>::max(), '\n');
					 cin >> ans;
					 cout
					 << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
					 */
					ans[0] = 'y';
					if (ans[0] == 'y') {
						agame.setCell((int) (isNotX(gameCh)),
								result.cellPtr->rowVal, result.cellPtr->colVal,
								E_NEAR);
					} else {
						char ccol;
						int row;
						sscanf(ans, "%d%c", &row, &ccol);
						int col = ccol - 'a' + 1;
						cout << row << " " << ccol << " "
								<< (int) (isNotX(gameCh)) << endl;
						row = reverseMapping(row);

						agame.setCell((int) (isNotX(gameCh)), row, col, E_NEAR);
					}
					//ahash.print();
					result = agame.evalAllCell(isNotX(gameCh), 20, -1, 0,
							maximizingPlayer,
							NINF, INF, debugThis, redonext, nullptr, &aTracer);
					//agame.print(SYMBOLMODE);
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
