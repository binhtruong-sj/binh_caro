/*
 * caro.h
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#ifndef CARO_H_
#define CARO_H_

#define oneD
//#define old
#define ROWCOL 1
#define SEast 7
#define South 6
#define SWest 5
#define West 4
#define NWest 3
#define North 2
#define NEast 1
#define East 0

#define INF(depth) {0x0FFFFFFF,0,depth}
#define NINF(depth) {0,0x0FFFFFFF,depth}

#define E_FAR 0 // cell val: empty, far from any occupied cell (4 too far)
#define BOUNDARY 0xb // for boundary cells
#define X_ 0x1    // occupied with X
#define O_ 0x2    // occupied with O
#define isX_(a) a==X_
#define isO_(a) a==O_

#define E_NEAR 0x10    // Empty cell, but close to other occupied cells
#define E_FNEAR 0x11    // Empty cell, but close to other occupied cells
#define E_TNEAR 0x20    // for temporary Near, clear after move is made
#define E_CAL 0x100
#define E_LAST 0x8000

#define st_RESET 0
#define st_SETUP 1
#define st_HUMAN_MOVE 2
#define st_GEN_WORKS 3
#define st_AI_MOVE 4

#define printInterval 100000
#define MAXDEPTH 41

#define SEARCH_DISTANCE 7
#define ReverseDirection(a) (a+4)%8
//#define oppositeVal(a) a^0x3
#define oppositeVal(a) a^0x3
#define myTurn(a) a%2
#define FLIP(a) ((a=a^1)? "ON":"OFF")
#define percentAdjust(val,percent) (val*percent)/100

#define MAXWIDTH 50
#define CONTINUOUS_BIAS 1

#define SYMBOLMODE 0
#define SYMBOLMODE2 2
#define SYMBOLMODE3 4
#define SYMBOLMODE4 12

#define SCOREMODE 6

#define SCOREMODEm 8
#define POSSMOVEMODE 10
#define cellVal(rowcol) board[rowcol.row][rowcol.col].val
#define cellScore(rowcol) board[rowcol.row][rowcol.col].score

#define toBinary(v,bianry) {int vvall = v;\
								for(int bit=7; bit>=0; bit--) {\
										binary[bit] = (vvall & 0x1) + '0';\
										vvall >>= 1;\
								}\
								binary[8]= 0;}
#define MIN(a,b) (a<b) ? a:b
#define MAX(a,b) (a>b) ? a:b
#define mapping(row) 16-row
#define reverseMapping(row) 16-row

#define asMIN(cmp,a,b) ((cmp = b.greaterValueThan(a))) ? a:b
#define asMAX(cmp,a,b) ((cmp = a.greaterValueThan(b))) ? a:b

#define convertToChar(setVal) (setVal == X_ ? 'X' : 'O')
#define convertCharToCol(col) ((char)(col-1+'a'))
#define convertCellToStr(val) (char) ((val == X_) ? 'X' : \
                              (val == O_) ? 'O' :\
		                   (val == E_FAR) ? '.' :\
		                     (val == 0xb) ? 'b' :\
		                     ( val & E_CAL)? '_':\
                          (val & E_TNEAR) ? '+' :\
                                              '`')
#define pause() {char ach; cin >> ach;}

/*
 * Line for the purpose of scoring
 */
using std::hex;
using std::showbase;

typedef int points;
struct hEntry {
public:
	int line;
	int connected;
	int bitcnt;
	int score;
	unsigned refcnt = 0;
};

class hashTable {
public:
	hEntry arrayE[1000];
	int arrayE_cnt = 0;
	unsigned lowest;
	int lowestI;
	int swapcnt = 0;
	hashTable();
	void swap2e(int from, int to);
	void addEntry(int line, int connected, int cnt, int score);
	void print();
};
#define COSTADJUSTPERCENT 50 // 5 percent cost per level of depth, lose 50% value after 10 level

class aScore {
public:
	points myScore = 0, oppScore = 0;
	int connectedOrCost = 0;
	/*
	 aScore(int m, int o, int c) {
	 myScore = m;
	 oppScore = o;
	 connectedOrCost = c;
	 }*/
	bool operator ==(const aScore & v) {
		bool out;
		out = (v.myScore == myScore) && (v.oppScore == oppScore)
				&& (v.connectedOrCost == connectedOrCost);
		return out;
	}
	friend ostream & operator <<(ostream & out, const aScore & v) {
		out << "(" << hex << v.myScore << "." << v.oppScore << "," << dec
				<< v.connectedOrCost << ")";
		return out;
	}

	aScore & operator =(const aScore & v) {
		myScore = v.myScore;
		oppScore = v.oppScore;
		connectedOrCost = v.connectedOrCost;
		return *this;
	}

	aScore & operator +=(const aScore & v) {
		myScore = myScore + v.myScore;
		oppScore = oppScore + v.oppScore;
		connectedOrCost = MAX(connectedOrCost, v.connectedOrCost);
		return *this;
	}
	aScore & init(int v, int d) {
		myScore = v;
		oppScore = d;
		connectedOrCost = 0;
		return *this;
	}

	int bestMove() {
		return myScore + oppScore;
	}
	int bestValue() {
		return myScore - oppScore;
	}

	bool greaterValueThan(aScore &other) {
		bool result;
		int deltaCost = other.connectedOrCost - connectedOrCost;
		if (deltaCost == 0) {
			result = (bestValue() > other.bestValue());
		} else if (deltaCost > 0) {
			int v = bestValue();
			v = v >> deltaCost;
			result = (v > other.bestValue());
		} else {
			int v = other.bestValue();
			v = v >> -deltaCost;
			result = (bestValue() > v);
		}
		return result;
	}
};
class Line {
public:
	friend class hashTable;
	int score;
	int val = 0; // 32 bit encoded of a line
				 // not the whole line.  Just started on O_
				 // scan to maximum of 7 squares
	int cnt = 0; // how many X_
	int connected; // How long is the longest connected X_
	int continuous;
	int blocked; // is it blocked by O_ (next to an X_).
	int offset; // to add or subtract to scoring
	int type; // X_ or O_ are being search

	friend ostream & operator <<(ostream & out, Line & v) {
		char binary[9];
		toBinary(v.val, binary);
		out << "Line=" << binary << " Cnt=" << v.cnt;
		out << " Blocked=" << v.blocked << " Connected=" << v.connected
				<< " cont=" << v.continuous;
		out << " Score = " << hex << v.score;
		return out;
	}

	bool operator ==(Line & other) {
		bool rtnval;
		rtnval = true;
		if (val != other.val) {
			cout << "val diff";
			rtnval = false;
		}
		if (score != other.score) {
			cout << "score diff";
			rtnval = false;
		}

		if (connected != other.connected) {
			cout << "connected diff";
			rtnval = false;

		}
		if (blocked != other.blocked) {
			cout << "blocked diff";
			rtnval = false;

		}
		if (offset != other.offset) {
			cout << "offset diff";
			rtnval = false;

		}
		if (continuous != other.continuous) {
			cout << "continuous diff";
			rtnval = false;

		}
		if (cnt != other.cnt) {
			cout << "cnt diff";
			rtnval = false;

		}
		return rtnval;
	}
	int evaluate(bool ending);
	void print() {
		char binary[9];
		toBinary(val, binary);
		printf("Line=%s Cnt=%d blocked=%d connected =%d", binary, cnt, blocked,
				connected);
		cout << hex << score;
		cout << endl;
	}
};

/*
 * composite score for all crosslines
 */
class FourLines {
public:
	Line Xlines[4];
	int score;
	void print();
};

/*
 * a cell in a caro table, has pointers to adjacent cells
 */
class cellV {
public:
	int val = 0;
	aScore score;
	friend ostream & operator <<(ostream & out, cellV& v) {
		out << convertCellToStr(v.val);
		return out;
	}
};
class cell: public cellV {
public:
	int rowVal, colVal;
	cell *near_ptr[8];
	void print() {
		cout << val;
	}

	void print(int v, int num) {
		if (num < 0) {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
		} else {
			char ach = (char) convertCellToStr(v);
			printf("%2d%c ", num, ach);
		}
		return;
	}

	void print(int mode) {
		int pval = val & 0xFFF;
		if (pval & (X_ | O_)) {
			if ((mode % 2) == 0) {
				char ach = (char) convertCellToStr(pval);
				if (val & E_LAST)
					printf(" <%c>", ach);
				else
					printf("%3c ", ach);
			} else {
				printf("%3c ", ' ');
			}
			return;
		}

		switch (mode) {
		case SYMBOLMODE: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE + 1): {
			return;
		}
		case SYMBOLMODE2: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE2 + 1): {
			pval = score.myScore + score.oppScore;
			break;
		}
		case SYMBOLMODE3: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE3 + 1): {
			pval = score.myScore - score.oppScore;
			break;
		}
		case SYMBOLMODE4: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (SYMBOLMODE4 + 1): {
			pval = val;
			break;
		}
		case SCOREMODE: // aiPlay
			pval = score.myScore;
			break;
		case SCOREMODE + 1: // opnVal or defVal
			pval = score.oppScore;
			break;
		case SCOREMODEm: // +
			pval = score.myScore + score.oppScore;
			break;
		case SCOREMODEm + 1: // -
			pval = abs(score.connectedOrCost);
			break;
		case POSSMOVEMODE: {
			char ach = (char) convertCellToStr(val);
			printf("%3c ", ach);
			return;
		}
		case (POSSMOVEMODE + 1): {
			return;
		}
		default:
			break;
		}
		if ((val & E_CAL) == 0) {
			pval = 0;
		}
		if (pval)
			if (pval > 0xFFF)
				cout << "FFF ";
			else
				printf("%3x ", (0xFFF & (pval)));
		else {
			if ((mode % 2) == 0)
				printf("  . ");
			else
				printf("    ");
//TODO: y defval printed on X 9j
		}
	}

	void printid() {
		printf("[%d%c]", mapping(rowVal), convertCharToCol(colVal));
	}

	friend ostream & operator <<(ostream &out, cell &c) {
		out << "[" << dec
				<< mapping(
						c.rowVal) << convertCharToCol(c.colVal) << "]" << convertCellToStr(c.val);
		return out;
	}
	cell & operator =(cell &c) {
		score = c.score;
		val = c.val;
		return *this;
	}

	bool operator ==(cell &other) {
		return ((other.score == score) && (other.val == val));
	}
};

class scoreElement: public aScore {
public:
	cell *cellPtr = nullptr;
	friend ostream & operator <<(ostream &output,
			vector<scoreElement> &values) {
		for (auto const& value : values) {
			output << *value.cellPtr << value.myScore << "," << value.oppScore
					<< "," << dec << value.connectedOrCost << "=" << dec
					<< value.myScore - value.oppScore << dec << "|"
					<< value.myScore - value.oppScore << endl;
		}
		return output;

	}
	friend ostream & operator <<(ostream &out, scoreElement &c) {
		if (c.cellPtr) {
			out << *c.cellPtr;
			out << hex << (c.myScore) << "," << hex << (c.oppScore) << ","
					<< dec << c.connectedOrCost << "=" << dec
					<< c.myScore - c.oppScore << dec << "|"
					<< c.myScore + c.oppScore << " ";
		} else
			cout << "NULL";
		return out;
	}
	scoreElement & operator=(const scoreElement & other) {
		myScore = other.myScore;
		oppScore = other.oppScore;
		connectedOrCost = other.connectedOrCost;
		cellPtr = other.cellPtr;
		return *this;
	}

	void getScore(const scoreElement & other) {
		myScore = other.myScore;
		oppScore = other.oppScore;
		connectedOrCost = other.connectedOrCost;
	}
	scoreElement & operator=(const aScore & other) {
		myScore = other.myScore;
		oppScore = other.oppScore;
		connectedOrCost = other.connectedOrCost;
		return *this;
	}

	scoreElement & operator=(cell & other) {
		cellPtr = &other;
		return *this;
	}
	scoreElement & score(scoreElement& f) {
		myScore = f.myScore;
		oppScore = f.oppScore;
		return *this;
	}
	points bestMove() {
		return myScore + oppScore;
	}
	points bestValue() {
		return myScore - oppScore;
	}
	bool greaterMove(scoreElement &other) {
		return (bestMove() > other.bestMove());
	}
	int getScore(int setVal, int myval) { // returning setVal's score, need aiPlay for ref
		return ((setVal == myval) ? myScore : oppScore);
	}
};

class tScore: public scoreElement {
public:
	aScore ts_ret;
	tScore();
	tScore(scoreElement);
};

class hist {
public:
	cell *hArray[100];
	int size;
	int gameCh;
	friend ostream & operator <<(ostream &out, const hist&c) {
		for (int i = 0; i < c.size; i++) {
			out << *c.hArray[i] << "->";
		}
		out << endl;
		return out;
	}

	int locate(int row, int col) {
		for (int i = 0; i < size; i++) {
			if ((row == hArray[i]->rowVal) && (col == hArray[i]->colVal))
				return (i);
		}
		return (-1);
	}
};

class acrumb: public cellV {
public:
	int width_id, depth_id;

	cell *ptr = nullptr;
	friend ostream & operator <<(ostream &out, const acrumb &c) {
		out << "{w=" << c.width_id << " d=" << c.depth_id << " ";
		if (c.ptr) {
			c.ptr->printid();
			out << convertCellToStr(c.val);
		} else
			out << "NULL ";
		cout << "}";
		return out;
	}
};
class breadCrumb {
public:
	acrumb top;
	acrumb *array;
	breadCrumb(int depth) {
		top.depth_id = depth;
		array = new acrumb[depth];
	}
	~breadCrumb() {
		delete array;
	}
	friend ostream & operator <<(ostream &out, const breadCrumb&c) {
		out << c.top << " | ";
		for (int i = c.top.depth_id - 1; i >= 0; i--) {
			out << c.array[i] << " - ";
		}
		out << endl;
		return out;
	}

	void extractTohistArray(hist & histArray) {
		int j = 0;
		histArray.gameCh = top.val;
		histArray.hArray[j++] = top.ptr;
		for (int i = top.depth_id - 1; i >= 0; i--) {
			if (array[i].ptr)
				histArray.hArray[j++] = array[i].ptr;
			else
				break;
		}
		histArray.size = j;
	}

	breadCrumb & operator=(const breadCrumb & other) {
		if (other.top.depth_id == top.depth_id) {
			top = other.top;
		} else if (other.top.depth_id <= top.depth_id - 1) {
			int dd =
					(other.top.depth_id < top.depth_id) ?
							other.top.depth_id : top.depth_id;
			for (int i = 0; i < dd; i++) {
				array[i] = other.array[i];
//				cout << "cc" << array[i];
			}
			array[dd] = other.top;
//			cout << "crumb=" << array[dd] << endl;
		} else {
			cout << "ERRROOOOROOOOR" << endl;
		}
		return *this;
	}
	int bestWidthAtDepth(int depth) {
		if (depth == top.depth_id)
			return top.width_id;
		else
			return array[depth].width_id;
	}
	cell * bestCellAtDepth(int depth) {
		if (depth == top.depth_id)
			return top.ptr;
		else
			return array[depth].ptr;
	}
};
class cellDebug {
	cell *debugCell[40];
	int dcDepth[40];
	int dcDline[40];
	int debugCnt = 0;

public:
	void reset() {
		debugCnt = 0;
	}
	void add(cell * dcell, int depth, int debugLine) {
		if (dcell) {
			cout << "add entry to cdb cnt=" << debugCnt << *dcell << endl;

			debugCell[debugCnt] = dcell;
			dcDepth[debugCnt] = depth;
			dcDline[debugCnt] = debugLine;
			debugCnt++;
		}
	}
	bool ifMatch(cell * lookupCell, int depth, int dcl) {
		for (int i = 0; i < debugCnt; i++) {
			if (depth < 0) {
				if ((dcl == dcDline[i]) && (*lookupCell == *debugCell[i]))
					return true;
			} else if ((dcl == dcDline[i]) && (depth == dcDepth[i])
					&& (*lookupCell == *debugCell[i]))
				return true;
		}
		return false;
	}
};
/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */
class caroDebug {
	vector<char> debugTrace, trace;
	int drow, dcol;
	/*
	 *
	 */
public:

	int printTrace() {
		cout << "\nTRACE: ";
		for (unsigned int i = 0; i < trace.size(); i++)
			printf("%02d,", trace[i]);
		return (trace.size());
	}
	/*
	 *
	 */
	void enterDebugTrace(char * debugString) {
		char *dstr;
		dstr = debugString;
		debugTrace.clear();
		while (char achar = *dstr++) {
			if (achar != ',') {
				debugTrace.push_back((points) (achar - '0'));
			}
		}
	}
	void enterDebugCell(char *debugString) {
		char ccol;
		sscanf(debugString, "[%d%c]", &drow, &ccol);
		if (isupper(ccol))
			dcol = ccol - 'A';
		else
			dcol = ccol - 'a';

		cout << "Debug info, row =" << drow << " col=" << dcol << endl;
	}
	bool traceMatch(int r, int c) {
		bool cellmatch = false;
		if ((drow >= 0) && (dcol >= 0)) {
			if ((r == drow) && (c == dcol))
				cellmatch = true;
		} else {
			cellmatch = true;
		}

		if (cellmatch) {
			if (trace == debugTrace) {
				cout << "---------------TRACE MATCH  row=" << r << " col=" << c
						<< endl;
				return true;
			}
		}

		return false;
	}
	void tracePush(int i) {
		trace.push_back((points) i);
	}

	void tracePop() {
		return (trace.pop_back());
	}

}
;
class debugid {
public:
	vector<int> id;
	bool find(int i) {
		bool debugNext = false;
		for (unsigned int ii = 0; ii < id.size(); ii++) {
			if ((debugNext = (id[ii] == i))) {
				break;
			}
		}
		return debugNext;
	}
};

class tracer {
public:
	tracer *prev = nullptr, *next = nullptr;
	scoreElement savePoint;
	int atDepth;
	unsigned int evalCnt = 0;

	tracer() {
		savePoint.cellPtr = nullptr;
	}
	tracer(cell *incptr) {
		savePoint.cellPtr = incptr;
	}
	~tracer() {
		// tracer is a link list. When delete, need to check if it point to a link list
		// if so, need to delete the entire link list
		tracer *toDel;
		toDel = next; // save the next ptr before deleteSelf.
		//	cout << savePoint << endl;
		next = nullptr;
		prev = nullptr;
		if (toDel) {
			delete toDel;
		}
	}
	friend ostream & operator <<(ostream & out, tracer & v) {
		tracer *tv = &v;
		cout << "\nTrC: ";
		do {
			if (tv) {
				cout << tv->atDepth << tv->savePoint << "#" << tv->evalCnt
						<< "->";
				tv = tv->next;
			} else
				break;
		} while (1);
		cout << endl;
		tv = &v;
		while (1) {
			if (tv) {
				cout << tv->atDepth << tv->savePoint << "#" << tv->evalCnt
						<< "<-";
				tv = tv->prev;
			} else
				break;
		}
		return out;
	}

	void print(FILE* ofile) {
		tracer *tv = this;

		fprintf(ofile, "\nTrc: ");
		do {
			if (tv) {
				fprintf(ofile, "[%d%c] ->", tv->savePoint.cellPtr->rowVal,
						(char) (tv->savePoint.cellPtr->colVal + 'a'));
				tv = tv->next;
			} else
				break;
		} while (1);
		fprintf(ofile, "\n");
	}
	void extractTohistArray(int val, cell * cptr, hist & histArray) {
		int j = 0;
		tracer *tracerPtr;
		histArray.gameCh = val;
		histArray.hArray[j++] = cptr;
		histArray.hArray[j++] = savePoint.cellPtr;
		tracerPtr = next;
		while (1) {
			if (tracerPtr == nullptr)
				break;
			if (tracerPtr->savePoint.cellPtr) {
				histArray.hArray[j++] = tracerPtr->savePoint.cellPtr;
			} else
				break;
			tracerPtr = tracerPtr->next;
		}
		histArray.size = j;
	}
};
class getInputStr {
	char inputstr[80];
	char *ptr = inputstr;
public:
	getInputStr() {
		clear();
	}
	void clear() {
		ptr = inputstr;
		inputstr[0] = 0;
	}

	int mygetstr(char *returnStr) {
		do {
			cout << "STRANGE-" << strlen(ptr);
			printf("%s-\n", ptr);
			int i = 0;
			if (*ptr) {
				if (!(isalpha(*ptr))) {
					while (isdigit(*ptr) || (*ptr == '-') || (*ptr == ',')) {
						if (*ptr == ',') {
							ptr++;
							break;
						}
						returnStr[i++] = *ptr++;
					}
					returnStr[i] = 0;
				} else {
					while ((isalpha(*ptr))) {
						returnStr[i++] = *ptr++;
					}
					returnStr[i] = 0;
				}
				return (strlen(ptr));
			} else {
				cout << "getting input" << endl;
				cin.clear();
				cin.ignore(numeric_limits < streamsize > ::max(), '\n');
				cin >> inputstr;
				ptr = inputstr;
			}
		} while (1);
	}
};
class rowCol {
public:
	int row, col;

	rowCol() {
		row = col = 0;
	}
	rowCol(int r, int c) {
		row = r;
		col = c;
	}

	rowCol & operator=(const rowCol & other) {
		row = other.row;
		col = other.col;
		return *this;
	}

	rowCol & setv(int r, int c) {
		row = r;
		col = c;
		return *this;
	}

	void moveToDir(int rdir, int cdir) {
		row += rdir;
		col += cdir;
	}
	void moveToDir(rowCol & rcDir) {
		row += rcDir.row;
		col += rcDir.col;
	}
	void reverse() {
		row = -row;
		col = -col;
	}
	void setDirection(int dir) {
		switch (dir) {
		case East:
			setv(0, +1);
			break;
		case NEast:
			setv(-1, +1);
			break;
		case North:
			setv(-1, 0);
			break;
		case NWest:
			setv(-1, -1);
			break;
		case West:
			setv(0, -1);
			break;
		case SWest:
			setv(1, -1);
			break;
		case South:
			setv(1, 0);
			break;
		case SEast:
			setv(1, +1);
			break;
		}
	}
	/*
	 int cellVal() {
	 return board[row][col].val;
	 }
	 */
};
class moveHist {
public:
	cell * movesHist[256]; // fixed at 256, change to use remeainder if different setting
	int movesHistVal[256];
	int lastMoveIndex = 0;
	int saveLast2p = 0;
	int moveCnt = 0;
	friend ostream & operator <<(ostream &out, const moveHist&c) {
		for (int i = 0; i < c.lastMoveIndex; i++) {
			out << *c.movesHist[i] << "->";
		}
		out << endl;
		return out;
	}
	void histPrint() {
		cout << "lastMoveIndex=" << lastMoveIndex << endl;
		for (int i = 0; i < lastMoveIndex; i++) {
			cout << "i=" << i << *movesHist[i] << "->";
		}
		cout << endl;
	}
	void addMove(cell *b) {
		moveCnt++;
		lastMoveIndex = (lastMoveIndex + 1) & 0xFF; // wrap around -- circular
		saveLast2p = lastMoveIndex;
		movesHist[lastMoveIndex] = b;
		movesHistVal[lastMoveIndex] = b->val;
	}
	void addMove(int mvNum, cell *b) {
		int lm = mvNum;
		movesHist[lm] = b;
		movesHistVal[lm] = b->val;
		lastMoveIndex = moveCnt; // this is to force the entry .. only work if all the restore are done
		saveLast2p = moveCnt;
	}
	void extractTohistArray(int currVal, hist & histArray) {
		histArray.gameCh = currVal;
		int i = moveCnt;
		while (i) {
			histArray.hArray[i - 1] = movesHist[lastMoveIndex - (moveCnt - i)];
			i--;
		}
		histArray.size = moveCnt;
	}
}
;
class PathElement {
public:
	scoreElement scoreE;
	bool isLeaf;
};
class Path {
public:
	PathElement array[30];
	void clear() {
		for (int i = 0; i < 30; i = i + 2) {
			array[i].scoreE = INF(i);
			array[i].isLeaf = 0;
		}
		for(int i = 1; i < 30; i=i+2) {
			array[i].scoreE = NINF(i);
			array[i].isLeaf = 0;
		}
	}
	void print(int i) {
		cout << "bestPath: ";
		do {
			cout << i << array[i].scoreE << "->";
		}while (array[i--].isLeaf== false);
		cout << "END" << endl;
	}
};
class caro: public moveHist {
public:
	int tid;
	caroDebug cdebug;
//	ofstream outfile;
//	FILE * ofile;
	vector<unsigned char> trace;
	Path bestPath; // track the best Path
	cell board[21][21];
	unsigned int evalCnt = 0;
	int size = 17;
	int maxDepth = 30;
	int my_AI_Play = X_;

	int localCnt = 0;
	int desiredRuntime = 0;
	cell* possMove[50];
	caro(int table_size) {
		size = table_size + 1;
		// Setup pointer to ajacent cells, only from 1 to size-1
		/*
		 #ifdef old
		 ofile  = fopen("oldCheckfile.txt","w");
		 #else
		 ofile  = fopen("Chkfile.txt","w");
		 #endif
		 */
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col].rowVal = row;
				board[row][col].colVal = col;
				if ((row == 0) || (col == 0) || (row == size)
						|| (col == size)) {
					board[row][col].val = BOUNDARY;
					for (int i = 0; i < 8; i++)
						board[row][col].near_ptr[i] = &board[row][col];
				} else {
					board[row][col].val = E_FAR; // FAR: empty cell 5 away from any occupied cell
					board[row][col].near_ptr[West] = &board[row][col - 1];
					board[row][col].near_ptr[NWest] = &board[row - 1][col - 1];
					board[row][col].near_ptr[North] = &board[row - 1][col];
					board[row][col].near_ptr[NEast] = &board[row - 1][col + 1];

					board[row][col].near_ptr[East] = &board[row][col + 1];
					board[row][col].near_ptr[SEast] = &board[row + 1][col + 1];
					board[row][col].near_ptr[South] = &board[row + 1][col];
					board[row][col].near_ptr[SWest] = &board[row + 1][col - 1];
				}
			}
		}
	}

	int printTrace() {
		cout << "\nTRACE: ";
		for (unsigned i = 0; i < trace.size(); i++)
			printf("%02de", trace[i]);
		return trace.size();
	}

	bool isMyPlay(int play) {
		return (play & my_AI_Play);
	}
	virtual ~caro();
	friend ostream & operator <<(ostream &out, const caro &c) {
		out << endl;
		for (int row = 0; row <= c.size; row++) {
			for (int col = 0; col <= c.size; col++) {
				out << " " << (char) convertCellToStr(c.board[row][col].val);
			}
			int rrr = 16 - row;
			out << " ROW " << rrr << endl;
		}
		out << "  ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			out << " " << pchar;
		out << endl;
		return out;
	}

	caro & operator =(caro & other) {
		size = other.size;
		maxDepth = other.maxDepth;
		my_AI_Play = other.my_AI_Play;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col] = other.board[row][col];
			}
		}
		return *this;
	}

	caro & save(caro & other) {
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				other.board[row][col] = board[row][col];
			}
		}
		return *this;
	}
	bool compare(caro & other) {
		bool result;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				if ((!(result = (other.board[row][col].score
						== board[row][col].score)))
						|| (!(result = (other.board[row][col].val
								== board[row][col].val)))) {

					cout << "FAILED  at row=" << row << " col=" << col << endl;
					cout << board[row][col] << board[row][col].score << " <> "
							<< other.board[row][col]
							<< other.board[row][col].score << endl;
					//return (false);
				}
			}
		}
		cout << "new";
		print(SYMBOLMODE2);
		cout << "backup";
		other.print(SYMBOLMODE2);
		return true;
	}

	void print(int mode) {

		int lastMoveIndex1 = lastMoveIndex - 1;

		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				if (movesHist[lastMoveIndex] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (movesHist[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
				board[row][col].print(mode);
				if (movesHist[lastMoveIndex] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (movesHist[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
			}
			cout << " ROW " << dec << mapping(row) << endl;
			for (int col = 0; col <= size; col++) {
				board[row][col].print(mode + 1);
			}

			cout << endl;
		}
		cout << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%3C ", pchar);
		cout << endl << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
				<< " Tid=" << tid << endl;
	}

	void print(hist &histArray) {
		int v, vb;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				int lh = histArray.locate(row, col);
				vb = board[row][col].val;
				if (vb & (X_ | O_))
					v = vb;
				else
					v = (lh % 2) ?
							oppositeVal(histArray.gameCh) : histArray.gameCh;
				board[row][col].print(v, lh);

			}
			cout << " ROW " << dec << mapping(row) << endl;
			cout << endl;
		}
		cout << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%3C ", pchar);
		cout << endl << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
				<< " Tid=" << tid << endl;

	}

	void print(cell* possMove[]) {
		int lastMoveIndex1 = lastMoveIndex - 1;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				if (movesHist[lastMoveIndex] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (movesHist[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
				int i;
				for (i = 0; i < 40; i++) {
					if (possMove[i] == nullptr) {
						i = 41;
						break;
					} else if (possMove[i] == &board[row][col]) {
						break;
					}
				}
				if (i >= 40)
					board[row][col].print(SYMBOLMODE);
				else
					printf("%3d ", i + 1);

				if (movesHist[lastMoveIndex] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (movesHist[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;

			}
			cout << " ROW " << dec << mapping(row) << endl;
			cout << endl;
		}
		cout << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%3C ", pchar);
		cout << endl << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
				<< " Tid=" << tid << endl;

	}

	void print() {
		cout << endl;
		for (int row = 0; row <= size; row++) {
			for (int col = 0; col <= size; col++) {
				board[row][col].print(0);
			}
			cout << " ROW " << mapping(row) << endl;
		}
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			printf("%2C ", pchar);
		cout << endl << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
				<< " Tid=" << tid << endl;

	}

	void undo1move() {
		cout << "Undo p=" << lastMoveIndex << "moveCnt=" << moveCnt << endl;
		cout << "saveLast2p=" << saveLast2p << endl;

		if ((lastMoveIndex - 2 != saveLast2p) && moveCnt > 0) {
			moveCnt -= 2;
			lastMoveIndex = (lastMoveIndex - 2) & 0xff;
			movesHistVal[lastMoveIndex + 2] = restoreCell(E_FAR,
					movesHist[lastMoveIndex + 2]->rowVal,
					movesHist[lastMoveIndex + 2]->colVal);
			movesHistVal[lastMoveIndex + 1] = restoreCell(E_FAR,
					movesHist[lastMoveIndex + 1]->rowVal,
					movesHist[lastMoveIndex + 1]->colVal);
		}
	}
	void redo1move() {
		cout << "Redo p=" << lastMoveIndex << "moveCnt=" << moveCnt << endl;
		cout << "saveLast2p=" << saveLast2p << endl;

		if (lastMoveIndex != saveLast2p) {
			moveCnt += 2;
			restoreCell(movesHistVal[lastMoveIndex + 2],
					movesHist[lastMoveIndex + 2]->rowVal,
					movesHist[lastMoveIndex + 2]->colVal);
			restoreCell(movesHistVal[lastMoveIndex + 1],
					movesHist[lastMoveIndex + 1]->rowVal,
					movesHist[lastMoveIndex + 1]->colVal);
			lastMoveIndex = (lastMoveIndex + 2) & 0xff;
		}
	}
	void reCalBoard(int currVal) {
		for (int row = 1; row < size; row++)
			for (int col = 1; col < size; col++) {
				if ((board[row][col].val & (O_ | X_)) == 0)
					score1Cell(currVal, row, col, false);
			}
	}

#define InspectDistance 4
#define saveRestoreDist InspectDistance+8 //  ---X---
	/*
	 * set a cell to X or O
	 */

	int setCell(int setVal, int row, int col, int near) {
		int oval = board[row][col].val;
		if ((oval & (X_ | O_)) == 0) { // this check is bc of lazyness of setting up board
			if (near == E_NEAR) { // Only real set can be UNDO
				// code to handle undo
				addMove(&board[row][col]);
				board[row][col].score = {moveCnt,0};
			} else if (near == E_FNEAR) { // Only real set can be UNDO
				moveCnt++;
				//		board[row][col].score = {moveCnt,0};
				board[row][col].score = {0,0};

			}
			else {
				localCnt++;
				//	board[row][col].score = {moveCnt+localCnt,0};
				board[row][col].score = {0,0};
			}
			board[row][col].val = setVal;
			if (near != E_FAR) {
				setNEAR(row, col, near & 0xF0);
			}
		}
		return oval;
	}

	/*
	 * After temporary setting a Cell to X' or O' for scoring, now return it and its neighbor to prev values
	 */

#ifdef ROWCOL
	int restoreCell(int saveVal, int row, int col) {
		rowCol crCurrCell, crDir;
		int rval;
		localCnt--;
		for (int dir = East; dir <= SEast; dir++) {
			crDir.setDirection(dir);
			crCurrCell.setv(row, col);
			for (int i = 0; i < InspectDistance; i++) {
				do {
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell)& (X_ | O_);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);

				cellVal(crCurrCell)= cellVal(crCurrCell) & ~(E_CAL);
				if (cellVal(crCurrCell)& (E_TNEAR)) {
					cellVal(crCurrCell) = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
				} else if (rval == 3) // boundary
				break;
			}
		}
		rval = board[row][col].val;
		board[row][col].val = saveVal & ~(E_CAL); // Return the val to prev
		return rval;
	}

	/*
	 * Temporary near marking set when traverse further ahead
	 */
	void setNEAR(int row, int col, int near) {
		rowCol crCurrCell, crDir;
		int rval;
		for (int dir = East; dir <= SEast; dir++) {
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			for (int i = 0; i < InspectDistance; i++) {
				do {
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell)& (X_ | O_);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				cellVal(crCurrCell)= cellVal(crCurrCell) & ~(E_CAL);
				if (cellVal(crCurrCell)==0) {
					cellVal(crCurrCell) = near;
				}
				if (rval == 3) // boundary
					break;
			}
		}
	}
	/*
	 *
	 */
	void saveScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		rowCol crCurrCell;
		rowCol crDir;
		int rval;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++) {
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			int j = 0;
			for (int i = 0; i < InspectDistance; i++) {
				do {
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell)& (X_ | O_);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				saveScoreArray[dir][j] = cellScore(crCurrCell);
				saveValArray[dir][j++] = cellVal(crCurrCell);
				if (rval == 3) // boundary
					break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		//cell *currCell;
		rowCol crCurrCell;
		rowCol crDir;

		int rval;
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
		for (int dir = East; dir <= SEast; dir++) {
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			int j = 0;
			for (int i = 0; i < InspectDistance; i++) {
				do {
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell)& (X_ | O_);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				cellScore(crCurrCell)= saveScoreArray[dir][j];
				cellVal(crCurrCell)= saveValArray[dir][j++];
				if (rval == 3) // boundary
					break;
			}
		}
	}
#else
	int restoreCell(int saveVal, int row, int col) {
		cell *currCell;
		int rval;
		localCnt--;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
					i = 0;
				}while ((rval != 3) && rval);
				currCell->val = currCell->val & ~(E_CAL);
				if (currCell->val & (E_TNEAR)) {
					currCell->val = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR

				} else if (rval == 3) // boundary
				break;
			}
		}
		rval = board[row][col].val;
		board[row][col].val = saveVal & ~(E_CAL); // Return the val to prev
		return rval;
	}

	void setNEAR(int row, int col, int near) {
		cell *currCell;
		int rval;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
					i = 0;
				}while ((rval != 3) && rval);
				currCell->val = currCell->val & ~(E_CAL);
				if (currCell->val == 0)
				currCell->val = near;
				if (rval == 3) // boundary
				break;
			}
		}
	}
	void saveScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		cell *currCell;
		int rval;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			int j = 0;
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
					i = 0;
				}while ((rval != 3) && rval);
				saveScoreArray[dir][j] = currCell->score;
				saveValArray[dir][j++] = currCell->val;
				if (rval == 3) // boundary
				break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
			aScore saveScoreArray[8][saveRestoreDist], aScore & s1Score,
			int saveValArray[8][saveRestoreDist], int & s1Val) {
		cell *currCell;
		int rval;
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
		for (int dir = East; dir <= SEast; dir++) {
			currCell = &board[row][col];
			int j = 0;
			for (int i = 0; i < InspectDistance; i++) {
				do {
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (X_ | O_);
					if (rval)
					i = 0;
				}while ((rval != 3) && rval);
				currCell->score = saveScoreArray[dir][j];
				currCell->val = saveValArray[dir][j++];
				if (rval == 3) // boundary
				break;
			}
		}
	}
#endif

	class traceCell {
	public:
		cell *cell;
		traceCell *prev;
		void extractTohistArray(hist & histArray) {
			int j = 0;
			traceCell *ptr;
			histArray.gameCh = cell->val;
			histArray.hArray[j++] = cell;
			ptr = prev;
			while (1) {
				if (ptr == nullptr)
					break;
				if (ptr->cell) {
					histArray.hArray[j++] = ptr->cell;
				} else
					break;
				ptr = ptr->prev;
			}
			histArray.size = j;
		}
	};

	void printDebugInfo(int row, int col, traceCell * trace, int depth) {
		cout << board[row][col];
//		printf("Play:%C",convertToChar(currPlay));
		printTrace();
		int i = depth;
		do {
			if (trace->cell)
				cout << i++ << *(trace->cell) << "->";
			trace = trace->prev;
		} while (trace);
		cout << endl;
	}
	void modifyDebugFeatures(int a);
	cell * inputCell() {
		int row, col;
		char ccol, inputstr[20];
		bool invalid = true;
		do {
			cout << "Enter cell:";
			cin >> inputstr;
			sscanf(inputstr, "%d%c", &row, &ccol);
			col = ccol - 'a' + 1;
			if (((row > 0) && (row < 16)) && ((col > 0) && (col < 16))) {
				return (&board[row][col]);
			}
		} while (invalid);
	}
	void clearScore();
	Line extractLine(int dir, int x, int y, bool &ending);
	aScore score1Cell(const int setVal, const int row, const int col,
			bool debugThis);

	scoreElement evalAllCell(int val, int width, int depth, int min_depth,
			bool maximizingPlayer, aScore alpha, aScore beta, bool debugThis,
			bool &redo, traceCell * const trace, tracer * const headTracer);

	scoreElement terminateScore;
	cellDebug cdbg;
	int nextWidth(int depth, int width) {
		int nw = width - (width / (depth + 1));
		if (nw <= 0)
			nw = 1;
		return (nw);
	}
	hist printTracerToBoard(int currPlay, const scoreElement& bestScore,
			tracer* headTracer);
	void reset();

private:
	int createNodeList(int thisWidth, int in_width,
			std::vector<scoreElement>& bestScoreArray, int& highestConnected,
			scoreElement& bestScore);
	int debugSetup(int thisWidth, int in_depth, int currPlay, bool topLevel,
			bool debugThis, int terminated,
			std::vector<scoreElement>& bestScoreArray, debugid& debug,
			std::vector<scoreElement>& returnScoreArray);

}
;

class briefHist {
	unsigned char *row, *col, *val, *order;
	int size = 2;
public:
	scoreElement result;
	briefHist() {
		size = 80;
		row = new unsigned char[size + 1];
		col = new unsigned char[size];
		val = new unsigned char[size];
		order = new unsigned char[size];
		row[0] = 0xff;
	}
	briefHist(int size) {
		row = new unsigned char[size + 1];
		col = new unsigned char[size];
		val = new unsigned char[size];
		order = new unsigned char[size];
		row[0] = 0xff;
	}
	~ briefHist() {
		delete row;
		delete col;
		delete order;
		delete val;
	}
	briefHist & operator =(const hist & c) {
		int i;
		for (i = 0; i < c.size; i++) {
			row[i] = (char) (c.hArray[i]->rowVal);
			col[i] = (char) (c.hArray[i]->colVal);
			val[i] = (char) c.hArray[i]->val;
			order[i] = (char) i;
		}
		row[i] = 0xFF;
		return *this;
	}

	briefHist & operator =(tracer * oc) {
		int i = 0;
		tracer *c = oc;
		do {
			row[i] = c->savePoint.cellPtr->rowVal;
			col[i] = c->savePoint.cellPtr->colVal;
			val[i] = (char) c->savePoint.cellPtr->val;
			order[i] = (char) i;
			i++;
			c = c->prev;
		} while (c);
		row[i] = 0xFF;

		return *this;
	}

	friend ostream & operator <<(ostream &out, const briefHist &c) {
		int i = 0;
		while (c.row[i] != 0xFF) {
			out << (int) c.order[i] << "[" << (int) mapping(c.row[i])
					<< (char) convertCharToCol(c.col[i]) << "]"
					<< convertToChar(c.val[i]) << "->";
			i++;
		}
		return out;
	}

	void addMove(int o, int r, int c, int v) {
		int i = 0;
		while (row[i++] != 0xFF)
			;
		i--;
		row[i] = r;
		col[i] = c;
		val[i] = v;
		order[i++] = o;
		row[i] = 0xFF;
	}
	void addMove(cell *aptr, int v) {
		int i = 0;
		while (row[i++] != 0xFF)
			;
		i--;
		row[i] = aptr->rowVal;
		col[i] = aptr->colVal;
		val[i] = v;
		order[i] = i;
		i++;
		row[i] = 0xFF;
	}
// setCells is for restoring board with previously play game
// with order maintained, FNEAR
	int setCells(caro * cr) {
		int i = 0;
		int mi = 0;
		while (row[i] != 0xFF) {
			int r = row[i];
			int c = col[i];
			int o = order[i];
			cr->addMove(o, &cr->board[r][c]);
			cr->setCell(val[i], r, c, E_FNEAR); // FNEAR
			if (o == 0)
				mi = i;
			i++;
		}
		return (row[mi]);
	}

	// this is mainly for setting cells with no order, and for temporary
	// play
	int setCellswithNoOrder(caro * cr, int *sA) {
		int i = 0;
		while (row[i] != 0xFF) {
			int r = row[i];
			int c = col[i];
			sA[i] = cr->setCell(val[i], r, c, E_TNEAR);
			i++;
		}
		return (row[0]);
	}
	void restoreCells(caro * cr, int *sA) {
		int i = 0;
		while (row[i] != 0xFF) {
			int r = row[i];
			int c = col[i];
			cr->restoreCell(sA[i], r, c);
			i++;
		}

	}
	void postResult(scoreElement & c) {
		result = c;
	}
}
;
#ifdef oneD
class oneWork {
public:
	cell* cptr;
	int val;
	scoreElement result;
	friend ostream & operator <<(ostream &out, oneWork &c) {
		out << *c.cptr;
		return out;
	}
	cell* setCellswithNoOrder(caro * cr, int *sA) {
		sA[0] = cr->setCell(val, cptr->rowVal, cptr->colVal, E_TNEAR);
		return (cptr);
	}
	void restoreCells(caro * cr, int *sA) {
		cr->restoreCell(sA[0], cptr->rowVal, cptr->colVal);
	}
	void postResult(scoreElement & c) {
		result = c;
	}
};
#else
class oneWork {
public:
	cell* cptr[2];
	int val[2];
	scoreElement result;
	friend ostream & operator <<(ostream &out, const oneWork &c) {
		out << *c.cptr[0] << "->" << *c.cptr[1];
		return out;
	}

	cell* setCellswithNoOrder(caro * cr, int *sA) {
		sA[0] = cr->setCell(val[0], cptr[0]->rowVal, cptr[0]->colVal, E_TNEAR);
		sA[1] = cr->setCell(val[1], cptr[1]->rowVal, cptr[1]->colVal, E_TNEAR);
		return (cptr[0]);
	}
	void restoreCells(caro * cr, int *sA) {
		cr->restoreCell(sA[0], cptr[0]->rowVal, cptr[0]->colVal);
		cr->restoreCell(sA[1], cptr[1]->rowVal, cptr[1]->colVal);
	}
	void postResult(scoreElement & c) {
		result = c;
	}
};
#endif

class worksToDo {
public:
	vector<oneWork> toDoList;
	unsigned int workTaken_index = 0;
	unsigned int workCompleted_index = 0;
	int cnt0 = 0, cnt1 = 0;
	int stageCnt = 0;
	int stage = 99;
	int threadCnt = 10;
	mutex mtx, stage_mtx, print_mtx, global_mtx, debug_mtx;
	int lasti;
	scoreElement global_alpha, global_beta;
	void update_alpha(aScore alpha) {
		if (alpha.greaterValueThan(global_alpha)) {
			global_mtx.lock();
			global_alpha = alpha;
			global_mtx.unlock();
		}
	}
	void update_beta(aScore beta) {
		if (global_beta.greaterValueThan(beta)) {
			global_mtx.lock();
			global_beta = beta;
			global_mtx.unlock();
		}
	}
	oneWork * getWork() {
		if (workTaken_index < toDoList.size()) {
			mtx.lock();
			oneWork *aptr = &toDoList[workTaken_index++];
			mtx.unlock();
			return aptr;
		} else
			return nullptr;
	}
	bool slaveId(int id) {
		return (id > 0);
	}

	/* stage:
	 * 99:reboot state
	 *  0: st_RESET
	 *  	reset stage;
	 *  	master: wait for all slave threads to inc cnt.
	 *  	Time-out and reset threadCnt to real
	 *  1: st_SETUP
	 *  	master: after read input; slave to copy agame.hist
	 *
	 *  2: st_HUMAN_MOVE
	 *  	master: after get input of human move
	 *     slaves: get input and calculate eval (-1),
	 *  3: st_GEN_WORKS
	 *  	master: after generate todolist
	 *     slaves: get works and complete it, until no-more work
	 *  4: st_AI_MOVE
	 *  	master: create final AI move
	 *     slave: copy final move
	 *
	 *  back to stage 2
	 *
	 */
	string stageName[5] = { "RESET", "READING INPUT", "Human-input",
			"WORK-distribution", " AI-Move" };

	void slaves_sync(int stageId, int sleep_ms) {
		while (stage != stageId) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
		stage_mtx.lock();
		cout << "Worker completing Stage " << stageName[stageId] << " Slave cnt="
				<< stageCnt << " id=" << stageId
				<< "------------------------------------------" << endl;
		stageCnt++;
		stage_mtx.unlock();
	}

	int master_setSync(int stageId, int sleep_ms, int timeout_ms) {
		// stuck in previous stage, wait for it sync up with everyone
#ifdef old
		return 0;
#else
		int r = timeout_ms / sleep_ms;
		stage_mtx.lock();
		stageCnt = 1;
		stage = stageId;
		stage_mtx.unlock();
		while ((stageCnt < threadCnt)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
			if (timeout_ms > 0)
				if (r-- < 0)
					return stageCnt;
		}
		cout << stage << " Master starting Stage= " << stageName[stageId]
				<< " cnt=" << stageCnt
				<< "------------------------------------------" << endl;
		return stageCnt;
#endif
	}

	void addWork(oneWork & awork) {
		toDoList.push_back(awork);
	}

	void clear() {
		mtx.lock();
		toDoList.clear();
		workTaken_index = workCompleted_index = 0;
		mtx.unlock();
	}

	int worksRemain() {
		return (toDoList.size() - workTaken_index);
	}

	scoreElement * getResult() {
		scoreElement *aptr;
		if (workCompleted_index <= workTaken_index) {
			aptr = &toDoList[workCompleted_index].result;
			/*cout << "HHHHHHHHH Comp=" << workCompleted_index << " Taken"
			 << workTaken_index << " "
			 << *toDoList[workCompleted_index].cptr << " result"
			 << *aptr;*/
			if ((volatile cell*) aptr->cellPtr) {
				workCompleted_index++;
				return (aptr);
			}
		}
		return nullptr;
	}

	bool allWorksCompleted() {
		/*
		 unsigned int lasti = 0;
		 if (workCompleted_index != lasti) {
		 cout << "WI=" << workTaken_index << " WCI=" << workCompleted_index
		 << endl;
		 lasti = workCompleted_index;
		 }
		 */
		return ((workTaken_index == (toDoList.size()))
				&& (workTaken_index == workCompleted_index));
	}

}
;
#endif /* CARO_H_ */
