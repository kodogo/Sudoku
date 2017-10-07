enum{
	Brdsize 	= 9,
	Psize 		= Brdsize * Brdsize,
	Alldigits 	= 0x1FF,
	Digit 		= 0x0000000F,
	Solve 		= 0x000000F0,
	Allow 		= 0x0001FF00,
	MLock 		= 0x00020000,

	Line 		= 0,
	Thickline 	= 1,
	Border 		= Thickline*4,
	Square 		= 48,
	Maxx 		= Square*9 + 2*Border,
	Maxy 		= Maxx + Square,
};

typedef struct Cell {
	int digit;
	int solve;
	int locked;
} Cell;

Cell	brd[Psize];

/* sudoku.c */
void drawbar(int digit, int selected);
void drawcell(int x, int y, int num, Image *col);
void drawblink(int cell);
char *genlevels(int i);

typedef struct Node Node;
typedef struct Col Col;

struct Node {
    Node *left;
    Node *right;
    Node *up;
    Node *down;
    Col *col; // May not be needed if make header row sparse-ish
};

struct Col {
    Node head; // Head of list of rows for this node. Not actually a row itself, just used for up and down links
    int len; // The number of rows currently in the column's list
    int index;
    // Don't call the next two left and right because the column list is circular
    Col *prev;
    Col *next;
};

int knuthsolve(Cell *board);

