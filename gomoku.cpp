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
#include <thread>
#include <mutex>
#include <sstream>

//#define PRIME
#define WaitTime 25
#define AUTO
using namespace std;
using namespace std::chrono;

#include "caro.h"
//#define convertCharToCol(a) isalpha(a)? (islower(a)? (a-'a'+1):(a-'A'+1)):1
#define isX(a) a=='X'?X_:O_
#define isNotX(a) a=='X'?O_:X_
extern int search_depth, search_width;
extern int debugScoring, debugScoringd, debugScoringAll, debugHash, docheck;
extern int underDebug, lowerMin, higherMin, training;
extern int interactiveDebug, lowerMinDepth, inspectCell;
extern int deltaDepth;
extern unsigned char iC[2];
int no_of_threads = 1;
int debugThis;
#ifdef AUTO
int autorun = 1;
#else
int autorun = 0;
#endif
caro agame(15), agame1(15), agame2(15), agame3(15), agame4(15), agame5(15),
		agame6(15), agame7(15);
worksToDo workQ;
briefHist bH;
int humanVal, aiVal;
int humanRow, humanCol, aiRow, aiCol;
int whichGame = 0;
void help() {
	cout << "-3 debugScoring " << endl
			<< "-4 debugBestPath specify path for debug, and at which depth to "
			<< endl << "display detail of line scoring " << endl
			<< "-11 debug line scoring" << endl
			<< "-23 debugScoringd, like best path, but for all instead" << endl
			<< "-24 display score for every move" << endl;

}
void getInputBriefHist(FILE *finput, caro &agame, briefHist & bh) {
	char aline[80], *cptr;
	int row, col;
	row = 1000;
	int order = 0;
	do {
		fgets(aline, 80, finput);
		cout << aline;

		cptr = aline;
		col = 0;
		while (char achar = *cptr++) {
			if ((achar != ' ') && (achar != '<') && (achar != '>')) {
				if ((row > 15) && (achar == 'b')) {
					row = 0;
					col = 0;
					break;
				} else if ((achar == 'L') || (achar == 'A')) {
					if (row == 1000)
						break;
					row = -100; // quit
					break;
				} else if (achar == 'b') {
					if (col > 1) {
						break;
					} else {
						row++;
						col = 0;
					}
				} else if (achar == 'X') {
					bh.addMove(order++, row, col, X_);
				} else if (achar == 'O') {
					bh.addMove(order++, row, col, O_);
				} else if (isdigit(achar)) {
					if ((row < 1) || (col < 1))
						break;
					cptr--;
					int num;
					char c;
					sscanf(cptr, "%d%c", &num, &c);
					while (*cptr++ != ' ')
						;
					if ((c == 'X') || (c == 'O')) {
						bh.addMove(num, row, col, isX(c));
					} else
						col--;
				}
				col++;
			}
		}
	} while (row >= 0);
	fgets(aline, 80, finput);
	cout << endl;
	agame.clearScore();
	cout << bh << endl;
	bh.setCells(&agame);
	hist histArray;
	agame.extractTohistArray(X_, histArray);
	agame.print(histArray);
}

void getInput(FILE *finput, caro &agame) {
	char aline[80], *cptr;
	int row, col;
	row = 1000;
	do {
		fgets(aline, 80, finput);
		cptr = aline;
		col = 0;
		while (char achar = *cptr++) {
			if ((achar != ' ') && (achar != '<') && (achar != '>')) {
				if ((row > 15) && (achar == 'b')) {
					row = 0;
					col = 0;
					break;
				} else if ((achar == 'L') || (achar == 'A')) {
					if (row == 1000)
						break;
					row = -100; // quit
					break;
				} else if (achar == 'b') {
					if (col > 1) {
						break;
					} else {
						row++;
						col = 0;
					}
				} else if (achar == 'X') {
					agame.setCell(X_, row, col, E_FNEAR);
				} else if (achar == 'O') {
					agame.setCell(O_, row, col, E_FNEAR);
				} else if (isdigit(achar)) {
					if ((row <= 1) || (col <= 1))
						break;
					cptr--;
					int num;
					char c;
					sscanf(cptr, "%d%c", &num, &c);
					while (*cptr++ != ' ')
						;
					cptr -= 2;
					agame.addMove(num, &agame.board[row][col]); // addMove need setCell right after
					col--;
				}
				col++;
			}
		}
	} while (row >= 0);
	fgets(aline, 80, finput);
	cout << endl;
	agame.clearScore();
	hist histArray;
	agame.extractTohistArray(X_, histArray);
	agame.print(histArray);
}

bool betterResult(oneWork &a, oneWork &b) {
	return (a.result.greaterValueThan(b.result));
}

scoreElement topLevel1(int val, int row, int col, int width, int depth,
		int min_depth, bool maximizingPlayer, aScore alpha, aScore beta,
		tracer * const aTracer) {
	bool redonext;
	/*	agame.evalAllCell(val, 20, -1, 0, maximizingPlayer, alpha, beta, 0,
	 redonext, nullptr, &aTracer);
	 */
	scoreElement result;

	agame.setCell(val, row, col, E_NEAR);

	agame.print(SYMBOLMODE2);

	// Get starting timepoint
	agame.evalCnt = 0;

	do {
		redonext = false;
		agame.localCnt = 0;
		aTracer->next = nullptr;
		result = agame.evalAllCell(oppositeVal(val), width, depth, min_depth,
				maximizingPlayer, alpha, beta, debugThis, redonext, nullptr,
				aTracer);
		aTracer->savePoint = result;
		if (redonext)
			delete aTracer->next;
	} while (redonext);
	return result;
}

scoreElement topLevel(int val, int row, int col, int width, int depth,
		bool maximizingPlayer, aScore alpha, aScore beta,
		tracer * const aTracer) {
	scoreElement * rResult, bestScore;
	bool redonext = false;
	workQ.clear();
	agame.setCell(val, row, col, E_NEAR);
	workQ.master_setSync(st_HUMAN_MOVE, 200, -1);
	workQ.global_alpha = alpha;
	workQ.global_beta = beta;
	agame.print(SYMBOLMODE2);

#ifdef oneD
	agame.evalAllCell(oppositeVal(val), width, -3, -3, maximizingPlayer, alpha,
			beta, debugThis, redonext, nullptr, aTracer);
#else
	agame.evalAllCell(oppositeVal(val), width, -2, -3, maximizingPlayer, alpha,
			beta, debugThis, redonext, nullptr, aTracer);
#endif
	scoreElement result;
	agame.print(SYMBOLMODE2);
	cout << "TOP" << endl;
	cout << agame << endl;

	/*
	 * SYNC
	 */

	workQ.master_setSync(st_GEN_WORKS, 200, -1);

#ifdef oneD
	bestScore = NINF(depth);
#else
	bestScore = INF(depth);
#endif
	int i = 0;
	while (workQ.allWorksCompleted() == false) {
		rResult = workQ.getResult();
		if (rResult) {
			cout << "I=" << i << workQ.toDoList[i] << " " << *rResult << "best="
					<< bestScore << endl;
			i++;
#ifdef oneD
			bool foundBetter = rResult->greaterValueThan(bestScore);
#else
			bool foundBetter = bestScore.greaterValueThan(*rResult);
#endif

			if (foundBetter)
				bestScore = *rResult;
		} else {
// not really waiting, just stalling
			/*	cout << "Master Waiting I=" << i << "taken="
			 << workQ.workTaken_index << "comp-"
			 << workQ.workCompleted_index << endl;*/
			std::this_thread::sleep_for(std::chrono::milliseconds(WaitTime));
		}
	}

	sort(workQ.toDoList.begin(), workQ.toDoList.end(), betterResult);
	for (unsigned int i = 0; i < workQ.toDoList.size(); i++) {
		cout << "BBB" << workQ.toDoList[i].result << endl;
	}
	return bestScore;
}
bool isPrime(int n) {
	// Corner case
	if (n <= 1)
		return false;

	// Check from 2 to n-1
	for (int i = 2; i < n; i++)
		if (n % i == 0)
			return false;

	return true;
}

void doPrime(int tid) {
	/*	cout << "WORKER" << endl;
	 cout << *acaro << endl;
	 */
	int cnt = 0;
	for (int i = 87628374; i < 97628374634; i++) {
		if (isPrime(i)) {
			if (cnt++ > 2)
				break;
		}
	}
	cout << "T=" << tid << "cnt" << cnt << endl;

}

/*
 * workerBee is slave threads -- all agame's data are used
 */
void workerBee(int tid) {
	tracer aTracer;
	caro * acaro;

	if (tid == 0)
		acaro = &agame;
	else if (tid == 1)
		acaro = &agame1;
	else if (tid == 2)
		acaro = &agame2;
	else if (tid == 3)
		acaro = &agame3;
	else if (tid == 4)
		acaro = &agame4;
	else if (tid == 5)
		acaro = &agame5;
	else
		acaro = &agame6;

// announcing your existence (setting up threadCnt)
	workQ.slaves_sync(st_RESET, 50);

	// getting data from inputfile
	workQ.slaves_sync(st_SETUP, 10);
	if (tid > 0) {
		agame.clearScore();

		bH.setCells(acaro);
	}
//
	int serialized = 0;
//	serialized = debugThis | 1;
	while (1) {
		workQ.slaves_sync(st_HUMAN_MOVE, WaitTime);
// getting a cell here
		acaro->maxDepth = 24;
		acaro->tid = tid;
		int depth = acaro->maxDepth;
		int min_depth = acaro->maxDepth - 12;
		bool maximizingPlayer = true;
		aScore alpha, beta;
		bool redonext = false;
		scoreElement result;

		result = acaro->evalAllCell(aiVal, 20, -1, 0, true, alpha, beta, 0,
				redonext, nullptr, &aTracer);
		if (tid > 0) {
			acaro->setCell(humanVal, humanRow, humanCol, E_NEAR);
			acaro->my_AI_Play = oppositeVal(humanVal);
		}
		//	acaro->compare(agame);
		workQ.slaves_sync(st_GEN_WORKS, WaitTime);
		/*	cout << "WORKER" << endl;
		 cout << *acaro << endl;
		 */
		if (tid >= no_of_threads) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		} else {
			while (workQ.worksRemain()) {
				for (unsigned int i = 0; i < 200; i++) {
					oneWork *workItem_ptr = workQ.getWork();
					if (workItem_ptr) {

						workQ.print_mtx.lock();
						cout << "T=" << tid << " Qsize = "
								<< workQ.toDoList.size() << " i=" << i
								<< *workItem_ptr << endl;
						workQ.print_mtx.unlock();

#ifdef PRIME
						doPrime(tid);
#else

						int saveArray[2];
						bool redonext;
						/*	acaro->print(SYMBOLMODE2);
						 cout << "BEFORE";*/
						result.cellPtr = workItem_ptr->setCellswithNoOrder(
								acaro, saveArray);
						//	acaro->print(SYMBOLMODE);
						if (serialized) {
							//				workQ.debug_mtx.lock();
						}
						int saveMax = acaro->maxDepth;
						int playVal;
						do {
							redonext = false;
							acaro->localCnt = 0;
							aTracer.next = nullptr;
							aTracer.atDepth = depth - 1;

							alpha = workQ.global_alpha;
							beta = workQ.global_beta;

							acaro->evalCnt = 0;

#ifdef oneD
							maximizingPlayer = false;
							playVal = humanVal;
							result.getScore(
									acaro->evalAllCell(playVal, 8, depth - 1,
											min_depth, maximizingPlayer, alpha,
											beta, debugThis, redonext, nullptr,
											&aTracer));
#else
							maximizingPlayer = true;
							playVal = aiVal;
							result.getScore(acaro->evalAllCell(playVal, 8,
											depth - 2, min_depth, maximizingPlayer, alpha,
											beta, debugThis, redonext, nullptr, &aTracer));
#endif
							/*	cout << "\nalpha=" << alpha << " Beta=" << beta
							 << " evalCnt= " << acaro->evalCnt << endl;
							 */
							aTracer.savePoint = result;
							if (redonext)
								delete aTracer.next;
						} while (redonext);
						//	cout << aTracer;
						//	aTracer.print(acaro->ofile);
						acaro->maxDepth = saveMax;
						acaro->printTracerToBoard(playVal, result, &aTracer);

						delete aTracer.next;
						if (serialized) {
							//			workQ.debug_mtx.unlock();
						}
						bool newAlpha, newBeta;
						if (maximizingPlayer == false) { // false, opposite with what given to evalAll.
							alpha = asMAX(newAlpha, result, alpha);
							if (newAlpha)
								workQ.update_alpha(alpha);
						} else {
							beta = asMIN(newBeta, result, beta);
							if (newBeta)
								workQ.update_beta(beta);
						}
						workItem_ptr->restoreCells(acaro, saveArray);
#endif
						/*	acaro->print(SYMBOLMODE2);
						 cout << "AFTER";*/
						//pause();
						cout << "Done with " << result << " Tid=" << tid
								<< endl;
						workItem_ptr->postResult(result);
						/*
						 workQ.print_mtx.lock();
						 cout << "Tid=" << tid << "RESULT=";
						 cout << workItem_ptr->result << endl;
						 workQ.print_mtx.unlock();
						 */
					} else
						break;
				}
			}
		}
		workQ.slaves_sync(st_AI_MOVE, WaitTime);
		acaro->setCell(aiVal, aiRow, aiCol, E_NEAR);

	}
}
int main() {

	int row, col, mode, dir;
	char name[4], testType[20];
	FILE *finput;
	Line tempLine;
	int depth = 5;
	bool redonext = false;
	float elapse = 0.0;

	bool maximizingPlayer = true;
	extern hashTable ahash;
	tracer aTracer;
	aTracer.prev = nullptr;
	aTracer.savePoint.cellPtr = &agame.board[1][1];
	aTracer.savePoint = {0,0,0};
	scoreElement result;

	string iline;
	//if (autorun) {
	ifstream myfile("input.txt");
	//}

//agame.print();
#if 1
	cout << "Enter no_of_threads deltaDepth " << endl;

	if (autorun == 0) {
		cin >> no_of_threads >> deltaDepth;
	} else {
		getline(myfile, iline);
		cout << iline << endl;
		stringstream(iline) >> no_of_threads >> deltaDepth;
	}

	depth = 2;

	thread th0(workerBee, 0);
	thread th1(workerBee, 1);
	thread th2(workerBee, 2);
	thread th3(workerBee, 3);
	thread th4(workerBee, 4);
	thread th5(workerBee, 5);
	thread th6(workerBee, 6);
	/*
	 */
// getting accurate threadCnt -- should be exactly
	/*
	 * SYNC
	 */
	workQ.threadCnt = workQ.master_setSync(st_RESET, 100, 2000);
	cout << "THREAD CNT" << workQ.threadCnt << endl;

	cout << "Enter filename " << endl;
	string aname;
	if (autorun == 0) {
		cin >> aname;
	} else {
		getline(myfile, iline);
		stringstream(iline) >> aname;
	}
	string fn = "testinput.txt";
	fn = fn + aname;
	cout << fn << endl;
	finput = fopen(fn.c_str(), "r");
	if (finput) {
		bool ending = false;

		getInputBriefHist(finput, agame, bH);
		agame.clearScore();
		/*
		 * SYNC
		 */
		workQ.master_setSync(st_SETUP, 200, -1);

		fscanf(finput, "%d", &mode);
		dir = 0;
		while (dir < 8) {
			if (mode)
				fscanf(finput, "%s %d %d %c", testType, &dir, &row, name);
			else
				scanf("%s %d %d %c", testType, &dir, &row, name);
			cout << "------------------" << testType << " " << dir << " " << row
					<< " " << name << endl;
			col = convertCharToCol(name[0]);
			int testc;
			if (testType[0] == '-') {
				sscanf(testType, "%d", &testc);
			} else {
				testc = testType[0];
			}

			switch (testc) {
			case -3:
				cout << "Turn " << FLIP(debugScoring) << " debugScoring"
						<< endl;

				break;
			case -4:
				cout << "Turn " << FLIP(underDebug) << " underDebug" << endl;

				break;
			case -5:
				cout << "Turn " << FLIP(lowerMin) << " lowerMin" << endl;

				break;

			case -6:
				cout << "Turn " << FLIP(higherMin) << " higherMin" << endl;

				break;
			case -7:
				cout << "Turn " << FLIP(lowerMinDepth) << " moreDepth" << endl;

				break;

			case 'q':
				break;
			case 'e':
				do {
					cout << "enter X dir row col" << endl;
					scanf("%s %d %d %c", testType, &dir, &row, name);
					col = convertCharToCol(name[0]);
					cout << row << " " << col << "name=" << name << testType
							<< endl;
					agame.setCell(isX(testType[0]), row, col, E_NEAR);
					agame.print(SYMBOLMODE);

					tempLine = agame.extractLine(dir, row, col, ending);
					tempLine.print();
					agame.restoreCell(0, row, col);

				} while (dir < 10);
				break;
			case '4': {
				FourLines astar;
				for (int dir = 0; dir < 4; dir++) {
					astar.Xlines[dir] = agame.extractLine(dir, row, col,
							ending);
				}
				astar.print();
				break;
			}
			case 's': {
				Line tempLine = agame.extractLine(dir, row, col, ending);
				tempLine.evaluate(ending);
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
				breadCrumb top_bc(depth);
				if (testType[1] == 'X')
					result = agame.evalAllCell(X_, width, depth, 0,
							!maximizingPlayer, NINF(depth), INF(depth), false,
							redonext, nullptr, &aTracer); // width = dir; depth = row
				else
					result = agame.evalAllCell(O_, width, depth, 0,
							!maximizingPlayer, NINF(depth), INF(depth), false,
							redonext, nullptr, &aTracer);
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
				break;

			case 'g':
				cout << "Opponent is White==O,  Black==X ;";
				cout << "Enter X/O:";
				char gameCh;
				if (autorun) {
					getline(myfile, iline);
					stringstream(iline) >> gameCh;
				} else
					cin >> gameCh;
				if (gameCh == 'O') { // X plays first
					humanVal = O_;
					aiVal = X_;
					agame.setCell(aiVal, 8, 8, E_NEAR);
				} else {
					humanVal = X_;
					aiVal = O_;
				}

				while (col < 20) {
					int redo = 1;
					char fans[10], ccol;
					do {
						agame.evalAllCell(humanVal, 20, -1, 0, maximizingPlayer,
						NINF(depth), INF(depth), 0, redonext, nullptr,
								&aTracer);
						cout << agame << endl;

						cout << "Result =" << result << "  ";
						cout << "Time taken by function: " << elapse
								<< " seconds" << endl;

						cout << "Enter row col" << endl;
						cout << "-1 undo -2 redo -3 debugScoring, -4 ("
								<< underDebug
								<< ") underDebug (turn off all Time limit logic),"
								<< "-5 (" << lowerMin << ") lowerMin " << "-6 ("
								<< higherMin << ")  higherMin " << "-7 ("
								<< lowerMinDepth << ") moreDepth "
								<< "-5 debugBestPath -6 debugAllPaths, -7 debugAI, "
								<< "-10 TRACE-debug -11 cells , "
								<< "-12 reset debug, -13 save -23 debugScoringd -24 debugScoringAll"
								<< " -25 debugHash -96 training -97 inspect-cell -98 docheck -99 interactiveDebug "
								<< " enter ? on col for details" << endl;
						if (autorun) {
							getline(myfile, iline);
							stringstream(iline) >> fans;
							cout << "-" << fans << "-" << endl;
						} else
							cin >> fans;

						if (fans[0] == 'q') {
							return (0);
						} else if (fans[0] == 's') {
							autorun = 0;
							cout << "switching input ";
							cin >> fans;
						}
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
							case -4:
								cout << "Turn " << FLIP(underDebug)
										<< " underDebug" << endl;

								break;
							case -5:
								cout << "Turn " << FLIP(lowerMin) << " lowerMin"
										<< endl;

								break;

							case -6:
								cout << "Turn " << FLIP(higherMin)
										<< " higherMin" << endl;

								break;
							case -7:
								cout << "Turn " << FLIP(lowerMinDepth)
										<< " moreDepth" << endl;

								break;
							case -96:
								cout << "Turn " << FLIP(training) << " training"
										<< endl;
								break;
							case -98:
								cout << "Turn " << FLIP(docheck) << " docheck"
										<< endl;

								break;
							case -97:
								cout << "Turn " << FLIP(inspectCell)
										<< " inspectCell" << endl;
								cout << "Enter cell:";

								scanf("%d%c", &row, &ccol);
								iC[0] = row;
								iC[1] = convertCharToCol(ccol);
								break;
							case -99:
								cout << "Turn " << FLIP(interactiveDebug) << "("
										<< interactiveDebug << ")"
										<< " interactiveDebug" << endl;
								debugThis = interactiveDebug;
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
						if (fans[0] == '?')
							help();

					} while (redo);
					int orow;

					if ((ccol == '=')) {
						cell* aptr = agame.possMove[row - 1]; // -1 to not use zero
						col = aptr->colVal;
						orow = aptr->rowVal;
						row = mapping(orow);
						cout << *aptr;
						printf(" AT possMove  row=%d col=%d\n", row, col);
					} else {
						col = ccol - 'a' + 1;
						orow = mapping(row);
					}
					if ((row > 15) || (col > 15)
							|| (agame.board[orow][col].val & (X_ | O_))) {
						cout << "Bad entry , val=" << hex
								<< agame.board[orow][col].val << endl;
						agame.print(SYMBOLMODE4);
						redo = 1;
					}
					cout << "fans=" << fans << " row=" << row << " col=" << col
							<< endl;
					debugThis = interactiveDebug;
					agame.maxDepth = 24;
					row = reverseMapping(row);
					agame.my_AI_Play = isNotX(gameCh);
					agame.trace.clear();
					auto start = high_resolution_clock::now();
					humanRow = row;
					humanCol = col;
					/*
					 * SYNC
					 */
					agame.bestPath.clear();
#ifdef old
					result = topLevel1(humanVal, row, col, 10, agame.maxDepth,
							agame.maxDepth - 10, maximizingPlayer,
							NINF(depth), INF(depth), &aTracer);
#else
					result = topLevel(humanVal, row, col, 10, agame.maxDepth,
							maximizingPlayer,
							NINF(depth), INF(depth), &aTracer);
#endif

					// Get ending timepoint
					auto stop = high_resolution_clock::now();
					auto duration = duration_cast<microseconds>(stop - start);
					elapse = ((float) duration.count() / 1000000.0);
					cout << "\nResult =" << result << "  ";
					cout << "Time taken by function: " << elapse << " seconds"
							<< endl;
					char ans[10];

					if (training) {
						cin >> ans;
					} else {
						ans[0] = 'y';
					}
					if (ans[0] == 'y') {
						agame.setCell((int) aiVal, result.cellPtr->rowVal,
								result.cellPtr->colVal,
								E_NEAR);
					} else {
						char ccol;
						int row;
						sscanf(ans, "%d%c", &row, &ccol);
						int col = ccol - 'a' + 1;
						cout << row << " " << ccol << " " << (int) aiVal
								<< endl;
						row = reverseMapping(row);

						agame.setCell((int) aiVal, row, col, E_NEAR);
					}
					cout << "History" << endl;
					hist histArray;
					agame.extractTohistArray(aiVal, histArray);
					cout << histArray;
					agame.print(histArray);
					//	cout << *aTracer.next;
					//	delete aTracer.next;
					aiRow = row;
					aiCol = col;
					/*
					 * SYNC
					 */
					workQ.master_setSync(st_AI_MOVE, 200, -1);
					//ahash.print();
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
