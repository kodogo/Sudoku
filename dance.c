//
//  main.c
//  dance
//
//  Created by ajs on 12/9/13.
//  Copyright (c) 2013 ajs. All rights reserved.
//

#include <u.h>
#include <libc.h>
#include <draw.h>
#include "sudoku.h"

#define MAXCOLS 325
#define MAXNODES 3300

Col cols[MAXCOLS];
Node nodes[MAXNODES];
int board [9][9];
Node *choice[100];

int row[9][9];
int col[9][9];
int box[9][9];
int nsolns;
int npuzzles;

jmp_buf env;

int colindex(int type, int major, int minor)
{
    return (type << 8) + (major << 4) + minor;
}

void addcol(Col *ccol, int index)
{
    ccol->index = index;
    ccol->head.up = &(ccol->head);
    ccol->head.down = &(ccol->head);
    ccol->len = 0;
    ccol->prev = ccol - 1;
    (ccol - 1)->next = ccol;
}

void makecols()
{
    // Makes the column nodes as a circularly linked list. cols[0] serves as the root node, but
    // does not represent a column itself
    int i, j;
    Col *ccol = cols + 1;;

    // Add in constraints that need covering in the solution
    for(i = 0; i < 9; i++)
        for(j = 0; j < 9; j++) {
            if(!board[i][j])
                addcol(ccol++, colindex(0, i, j));
            if(!row[i][j])
                addcol(ccol++, colindex(1, i, j));
            if(!col[i][j])
                addcol(ccol++, colindex(2, i, j));
            if(!box[i][j])
                addcol(ccol++, colindex(3, i, j));
        }
    
    // Now include the root column in the list
    ccol--;
    ccol->next = cols;
    cols[0].prev = ccol;
}

void addconstraint(Node *node, int type, int major, int minor)
{
    Col *ccol = cols + 1;
    int index;
    
    index = colindex(type, major, minor);
    // Don't need to test for termination because the column record is guaranteed to exist after calling makecols (?)
    while(ccol->index != index)
        ccol++;
    
    if(type != 0) {
        node->left = node - 1;
        (node - 1)->right = node;
    }
    node->col = ccol;
    node->up = ccol->head.up;
    ccol->head.up->down = node;
    ccol->head.up = node;
    node->down = &(ccol->head);
    ccol->len++;
}

void addrows()
{
    // Add each row containing 4 constraints
    
    int r, c, b, d;
    
    Node *node = nodes;
    for(r = 0; r < 9; r++)
        for(c = 0; c < 9; c++)
            if(!board[r][c]) {
                b = ((r / 3) * 3) + c / 3;
                for(d = 0; d < 9; d++) {
                    if(!row[r][d] && !col[c][d] && !box[b][d]) {
                        addconstraint(node++, 0, r, c); // Cell (r, c) is occupied
                        addconstraint(node++, 1, r, d); // Row r contains digit d
                        addconstraint(node++, 2, c, d); // Column c contains digit d
                        addconstraint(node++, 3, b, d); // Box b ccontains digit d
                        (node - 4)->left = node - 1;
                        (node - 1)->right = node - 4;
                    }
                }
            }    
}

int fillboard(Cell brd[81])
{
    int r, c, b, d;
    for(r = 0; r < 9; r++)
        for(c = 0; c < 9; c++) {
                d = brd[c*9 + r].digit;
            if(d >= 0) {
                if(row[r][d]) // Row r already contains digit d
                    return 0;
                row[r][d] = 1;
 
                if(col[c][d]) // Column c already contains digit d
                    return 0;
                col[c][d] = 1;

                b = ((r / 3) * 3) + (c / 3);
                if(box[b][d]) // Box b already contains digit d
                    return 0;
                box[b][d] = 1;
                
                board[r][c] = d + 1;
            }
        }
    return 1;
}

void cover(Col *c)
{
    // Remove c from the header list, and remove all rows in c from the other column lists
    // they are in
    Col *l, *r;
    Node *rr, *nn, *uu, *dd;
    
    // Remove c from column list by jiggling pointers, but don't physically delete    
    l = c->prev;
    r = c->next;
    l->next = r;
    r->prev = l;
    
    // Now iterate over rows in c
    for(rr = c->head.down; rr != (&c->head); rr = rr->down)
        // Iterate over nodes in row
        for(nn = rr->right; nn != rr; nn = nn->right) {
            // Hide node from column it's in
            uu = nn->up;
            dd = nn->down;
            uu->down = dd;
            dd->up = uu;
            nn->col->len--;
        }
}

void uncover(Col *c)
{
    
    // Undoes the cover operation by carrying out the operations in reverse order, restoring
    // the pointers to their previous state.
    Col *l, *r;
    Node *rr, *nn, *uu, *dd;
    
    for(rr = c->head.up; rr != &(c->head); rr = rr->up)
        for(nn = rr->left; nn != rr; nn = nn->left) {
            uu = nn->up;
            dd = nn->down;
            uu->down = nn;
            dd->up = nn;
            nn->col->len++;
        }
    
    l = c->prev;
    r = c->next;
    l->next = c;
    r->prev = c;    
}

Col *bestcol()
{
    // Returns the column with the smallest number of rows
    int minlen = MAXNODES;
    Col *c;
    Col *best;
    for(c = cols->next; c != cols; c = c->next)
        if(c->len < minlen) {
            best = c;
            minlen = c->len;
        }
    return best;
}

void coverall(Node *node)
{
    Node *pp;
    for(pp = node->right; pp != node; pp = pp->right)
        cover(pp->col);
}

void uncoverall(Node *node)
{
    Node *pp;
    for(pp = node->left; pp != node; pp = pp->left)
        uncover(pp->col);
}

void processrow(Node *node, int soln[9][9])
{
    Node *n;
    int type, index, r, c, d;
    n = node;
    do {
        index = n->col->index;
        type = index >> 8;
        if(type == 0) {
            r = (index >> 4) & 0xf;
            c = index & 0xf;
        }        
        if(type == 1)
            d = (index & 0xf) + 1;
        
        n = n->right;
    } while(n != node);
    soln[r][c] = d;
}


void savesoln(Cell brd[81], int k)
{
    int r, c;
    int soln[9][9];
    
    memcpy(soln, board, sizeof(board));
    for(r = 0; r < k; r++)
        processrow(choice[r], soln);
    for(r = 0; r < 9; r++)
        for(c = 0; c < 9; c++)
            if(soln[r][c] != 0)
                brd[c*9 + r].solve = soln[r][c] - 1;
}


void search(int k, Cell brd[81])
{
    Col *col;
    Node *node;
    
    if(cols->next == cols) {
            savesoln(brd, k);
            // At this point, just bail out at the first solution, regardless if it is not unique
            longjmp(env, 1);
    }
    
    col = bestcol();
    cover(col);
    for(node = col->head.down; node != &(col->head); node = node->down) {
        choice[k] = node;
        coverall(node);
        search(k + 1, brd);
        uncoverall(node);
    }
    uncover(col);
}


int knuthsolve(Cell brd[81])
{
    nsolns = 0;
    memset(board, 0, sizeof(board));
    memset(row, 0, sizeof(row));
    memset(col, 0, sizeof(col));
    memset(box, 0, sizeof(box));
    
    if(!fillboard(brd))
        return 0;
    makecols();
    addrows();   if(setjmp(env) == 0) {
        search(0, brd);
    }
    // Should really check for no solution found, but out of time for now
    return 1;
}
