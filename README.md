# Sudoku
Sudoku solver for Plan9 on Raspberry pi

The code is written in Plan9 C. 

It was written in something of a hurry as a demo for the codeworkx challenge:
	http://codeworkx.co.nz
	
The style is somewhat terse, in keeping with standard Plan9 practice.

The UI code was adapted from that in the standard Plan9 distro, 
with permission from the author:
	http://plan9.bell-labs.com/sources/plan9/sys/src/games/sudoku/
	
The solver treats a Sudoku puzzle as an exact cover problem,
and uses the algorithm described in Knuth's "Dancing Links" paper:
	www-cs-faculty.stanford.edu/~uno/papers/

The matrix for the exact cover problem has 729 rows and 324 columns.
Each row in the matrix corresponds to an assignment of one digit to one
square - hence 9 * 81 rows.

There are four groups of 81 columns. A “1” in a column specifies that a particular
condition is satisfied. For the first group of 81 columns, a “1” indicates that the
corresponding square contains a digit. For the second group of 81 columns, a “1” indicates
that a particular row contains a particular digit. The third and fourth groups do the
same job for the columns and “boxes”. Hence each row will contain exactly four “1”s.

The solution to the exact cover problem results in a set of 81 rows, each row
giving the assignment of a digit to a square. The presence of a “1” in each column
of the resulting matrix shows that all of the constraints required for a Sudoku solution 
have been met.

Files:

mkfile - Plan9 mkfile to build the program
sudoku.h - main include file
puzzles.h - a few hard-wired puzzles
sudoku.c - driver/UI code
dance.c - solver code

A video may be found here:
	https://vimeo.com/87051049
