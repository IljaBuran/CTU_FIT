#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

/* Defining initial size and resize for vectors */
#define Vec_Init_Size 10
#define Vec_Resize_Coef 2

size_t programmer_sin = 0;

/* 2D position */
typedef struct
{
    int column;
    int row;
}Position;

/* Function to create a position */
Position MakePosition(int column, int row)
{
    Position ret = {column, row};
    return ret;
}

/* Game entity (position and type) */
typedef struct
{
    Position pos;
    char type;
}Cell;

/* Function to create a Cell */
Cell MakeCell(Position pos, char c)
{
    Cell ret = {pos, c};
    return ret;
}

/* Function to compare 2 Positions */
bool EqualPositions(const Position* a, const Position* b)
{
    return (a->column == b->column && a->row == b->row);
}

/* 
    Struct of dynamically allocated array of Cells
        Cell* data -> array itself
        size_t count -> number of Cells in data
        size_t capacity -> maximum no. of Cells in data
        size_t jumps -> keeps track of eliminated black Cells
*/
typedef struct
{
    Cell* data;
    size_t count;
    size_t capacity;
    size_t jumps;
}Vec;

/* Functions to manage Vec struct */

/* Initializes Vec */
void Vec_init(Vec* v)
{
    v->capacity = Vec_Init_Size;
    v->data = (Cell*)malloc(v->capacity * sizeof(Cell));
    v->count = 0;
    v->jumps = 0;
}

void Vec_reset(Vec* v)
{
    v->count = 0;
    v-> jumps = 0;
}

/* Resizes array when maximum capacity reached */
void Vec_resize(Vec* v)
{
    if (v->capacity == v->count)
    {
        v->capacity *= Vec_Resize_Coef;
        v->data = (Cell*)realloc(v->data, v->capacity * sizeof(Cell));
    }
}

/* Assigning new cell at the end of array */
void Vec_pushback(Vec* v, const Cell* cell)
{
    Vec_resize(v);
    v->data[v->count++] = *cell;
}

/* Removes last element in array */
Cell Vec_popback(Vec* vec2d) 
{
    vec2d->count--;
    return vec2d->data[vec2d->count];
}

/* Checks if there is equivalent position in array */
bool Vec_find(const Vec* v, const Cell* cell)
{
    for (size_t i = 0; i < v->count; i++)
        if (EqualPositions(&v->data[i].pos, &cell->pos))
            return true;
    return false;
}

/* Function for qsort in Vec2d_sort */
int CmpFn(const void* a, const void* b)
{
    return tolower(((Cell*)b)->type) - tolower(((Cell*)a)->type);
}

/* Sorts Vec based on type */
void Vec_sort(Vec* v)
{
    qsort(v->data, v->count, sizeof(Cell), CmpFn);
}

/* Frees Vec */
void Vec_free(Vec* v)
{
    free(v->data);
}

/* 
    Struct of dynamically allocated gameboard 
        char** data -> stores size*size board
        int size -> size of board
*/
typedef struct
{
    char** data;
    int size;
}Vec2d;

/* Functions to manage Vec2d struct */

/* Initializes board, sets all positions to ' ' */
void Vec2d_init(Vec2d* board, int size)
{
    board->size = size;
    board->data = (char**)malloc(board->size * sizeof(char*));

    for (int i = 0; i < board->size; i++)
    {
        board->data[i] = (char*)malloc(board->size * sizeof(char));
        memset(board->data[i], ' ', board->size);
    }
}

/* Frees memory of board */
void Vec2d_free(Vec2d* board)
{
    for (int i = 0; i < board->size; i++)
        free(board->data[i]);
    free(board->data);
}

/* Assigns new Cell into board */
void Vec2d_addCell(Vec2d* board, const Cell* cell)
{
    board->data[cell->pos.row][cell->pos.column] = cell->type;
}

/* Helper functions */

/* Converts alphabet index of column into corresponding number */
int IndexCharToInt(char c)
{
    return (int)c - 97;
}
/* Converts number of column into corresponding alphabet index */
char IndexIntToChar(int n)
{
    return (char)n + 97;
}

/* Checks board size validity */
bool IsBoardSizeValid(int size) { return (size >= 3 && size <= 26); }

/* 
    Checks whether 2 index' are out of bounds of a board,
    order of a and b (column and row) doesn't matter -> board is square type
*/
bool CheckOutOfBounds(const Vec2d* vec, int a, int b){ return (a >= 0 && a < vec->size && b >= 0 && b < vec->size); }

/* Check stdin whether non-whitespace character is there prior to EOF or newline */
bool CheckBuffer()
{
    int c;
    while((c = getchar()) != '\n' && c != EOF)
        if (!isspace(c))
            return false;
    return true;
}

/* Checks whether position on input is valid based on board size and if it's black space */
bool IsPositionValid(int boardSize, char column, int row)
{
    int columnIndex = column - 96;
    if (row < 1 || row > boardSize || columnIndex < 1 || 
        columnIndex > boardSize || (columnIndex + row) % 2 != 0)
        return false;
    return true;
}

/* Checks input validity */
bool IsInputValid(int boardSize, char type, char column, int row)
{
    if (type != 'W' && type != 'w' && type != 'B' && type != 'b')
        return false;
    if (!IsPositionValid(boardSize, column, row))
        return false;
    return true;
}

/*
    Initializes position vector and board 2d vector,
    takes care of input and checks validity,
    assigns inputs into board
*/
bool HandleInput(Vec2d* board, Vec* positions)
{
    /* First message to user */
    printf("Velikost hraci plochy:\n");
    
    /* Size of board */
    int boardSize;
    if (scanf("%d", &boardSize) != 1 || !IsBoardSizeValid(boardSize) || !CheckBuffer())
        return false;
    
    Vec_init(positions);
    Vec2d_init(board, boardSize);

    /* First message to user */
    printf("Pozice kamenu:\n");
    
    /* Take input until EOF */
    while(true)
    {
        char type, column;
        int row;

        /* scanf input and check validity */
        int scanfOutput = scanf(" %c %c%d", &type, &column, &row);
        if (scanfOutput == EOF)
            return true;
        /* if not valid, free memory and returns false */
        if (scanfOutput != 3 || !IsInputValid(board->size, type, column, row))
        {
            Vec2d_free(board);
            Vec_free(positions);
            return false;
        }
        
        /* if input is valid, then assign into board */
        Cell temp = {{IndexCharToInt(column), row - 1}, type};
        if (Vec_find(positions, &temp))
        {
            Vec2d_free(board);
            Vec_free(positions);
            return false;
        }
        Vec_pushback(positions, &temp);
        Vec2d_addCell(board, &temp);
    }
}

/* prints result based on vec */
void PrintResult(Vec* vec)
{
    for (size_t i = 0; i < vec->count; i++)
    {
        printf("%c%d", IndexIntToChar(vec->data[i].pos.column), vec->data[i].pos.row + 1);
        if (i != vec->count - 1)
            printf(" -> ");
        else
        {
            if (vec->jumps > 0)
                printf(" +%ld", vec->jumps);//*
            printf("\n");
        }
    }
    programmer_sin++;
}

void BasicMovesJumped(Vec2d* board, Vec* results);

/* Prepares data and calls BasicMovesJumped */
void CallBasicMovesJumped(Vec2d* board, Vec* results, Position pos, int col_coef)
{
    board->data[pos.row - 1][pos.column - col_coef] = ' ';
    results->jumps++;
    Cell pass = MakeCell(pos, ' ');
    Vec_pushback(results, &pass);
    BasicMovesJumped(board, results);
    Vec_popback(results);
    results->jumps--;
    board->data[pos.row - 1][pos.column - col_coef] = 'b';
}

/* Logic of basic moves after jumping */
void BasicMovesJumped(Vec2d* board, Vec* results)
{
    PrintResult(results);
    
    /* DOWN_RIGHT */
    Position currPos = results->data[results->count - 1].pos;
    if (CheckOutOfBounds(board, currPos.column + 2, currPos.row + 2) && board->data[currPos.row + 2][currPos.column + 2] == ' ')
        if (board->data[currPos.row + 1][currPos.column + 1] == 'b' || board->data[currPos.row + 1][currPos.column + 1] == 'B')
        {
            Position pass = MakePosition(currPos.column + 2, currPos.row + 2);
            CallBasicMovesJumped(board, results, pass, +1);
        }
    /* DOWN_LEFT */
    if (CheckOutOfBounds(board, currPos.column - 2, currPos.row + 2) && board->data[currPos.row + 2][currPos.column - 2] == ' ')
        if (board->data[currPos.row + 1][currPos.column - 1] == 'b' || board->data[currPos.row + 1][currPos.column - 1] == 'B')
        {
            Position pass = MakePosition(currPos.column - 2, currPos.row + 2);
            CallBasicMovesJumped(board, results, pass, -1);
        }
}

/* Logic of basic moves before jumping */
void BasicMovesNonJumped(Vec2d* board, Vec* results, int coef_col)
{
    Position newPos = MakePosition(results->data[results->count - 1].pos.column + coef_col, results->data[results->count - 1].pos.row + 1);
    if (CheckOutOfBounds(board, newPos.column, newPos.row))
    {
        if (board->data[newPos.row][newPos.column] == ' ')
        {
            printf("%c%d -> %c%d\n", IndexIntToChar(results->data[0].pos.column), results->data[0].pos.row + 1, IndexIntToChar(newPos.column), newPos.row + 1);
            programmer_sin++;
        }
        else if (board->data[newPos.row][newPos.column] == 'b' || board->data[newPos.row][newPos.column] == 'B')
        {
            newPos.column += coef_col; newPos.row++;
            if (CheckOutOfBounds(board, newPos.column, newPos.row) && board->data[newPos.row][newPos.column] == ' ')
            {
                CallBasicMovesJumped(board, results, newPos, coef_col);
            }
        }
    }
}

/* algorithm for calculating all possible move of rock/stone/or wth it's supposed to be called in english :D */
void BasicMoves(Vec2d* board, Vec* results)
{
    /* DOWN_RIGHT */
    BasicMovesNonJumped(board, results, +1);
    /* DOWN_LEFT */
    BasicMovesNonJumped(board, results, -1);
}

/* 
    Checks board in direction based on parameters,
    if b/B is found, 
        checks if there's blank space behind, 
        sets pos to the blank space found and returns true
    else
        returns false;
*/
bool IsThereB(Vec2d* board, Position* pos, int coef_row, int coef_col)
{
    //printf("IsThereB\n");
    while(CheckOutOfBounds(board, pos->row, pos->column))
    {
        //printf("1");
        if (board->data[pos->row][pos->column] == ' ')
        {
            pos->row += coef_row; pos->column += coef_col; continue;
        }
        else if (board->data[pos->row][pos->column] == 'B' || board->data[pos->row][pos->column] == 'b')
        {
            if (CheckOutOfBounds(board, pos->row + coef_row, pos->column + coef_col) && board->data[pos->row + coef_row][pos->column + coef_col] == ' ')
            {
                pos->row += coef_row; pos->column += coef_col;
                return true;
            }
            else return false;
        }
        else break;
    }
    return false;
}

void QueenMovesJumped(Vec2d* board, Vec* results, int coef_row, int coef_col);

/* Function prepares parameters and calls QueenMovesJumped */
/* basically created to not repeat the code */
void CallQueenMovesJumped(Vec2d* board, Vec* results, Position pos, int coef_row, int coef_col, bool jump)
{
    //printf("CallQueenMovesJumped\n");
    //PrintResult(results);
    Cell pass = MakeCell(pos, ' '); Cell recovery;
    if (jump)
    {
        board->data[pos.row - coef_row][pos.column - coef_col] = ' ';
        results->jumps++;
    }
    else
        recovery = Vec_popback(results);

    Vec_pushback(results, &pass);
    QueenMovesJumped(board, results, coef_row, coef_col);
    Vec_popback(results);

    if (jump)
    {
        board->data[pos.row - coef_row][pos.column - coef_col] = 'b';
        results->jumps--;
    }
    else
        Vec_pushback(results, &recovery);
}

/* Check all possible moves for Queen after jumping */
void QueenMovesJumped(Vec2d* board, Vec* results, int coef_row, int coef_col)
{
    PrintResult(results);
    Position newPos;
    bool reverse = ((coef_row < 0 && coef_col > 0) || (coef_row > 0 && coef_col < 0));
    
    /* Look left for B\b */
    int dir_coef_row = reverse ? coef_row : coef_row * -1;
    int dir_coef_col = reverse ? coef_col * -1 : coef_col;

    newPos = MakePosition(results->data[results->count - 1].pos.column + dir_coef_col, results->data[results->count - 1].pos.row + dir_coef_row);
    if (IsThereB(board, &newPos, dir_coef_row, dir_coef_col))
        CallQueenMovesJumped(board, results, newPos, dir_coef_row, dir_coef_col, true);

    /* Look right for B\b */
    dir_coef_row = reverse ? coef_row * -1 : coef_row;
    dir_coef_col = reverse ? coef_col : coef_col * -1;

    newPos = MakePosition(results->data[results->count - 1].pos.column + dir_coef_col, results->data[results->count - 1].pos.row + dir_coef_row);
    if (IsThereB(board, &newPos, dir_coef_row, dir_coef_col))
        CallQueenMovesJumped(board, results, newPos, dir_coef_row, dir_coef_col, true);

    /* Look forward for B\b */
    newPos = MakePosition(results->data[results->count - 1].pos.column + coef_col, results->data[results->count - 1].pos.row + coef_row);
    if (IsThereB(board, &newPos, coef_row, coef_col))
        CallQueenMovesJumped(board, results, newPos, coef_row, coef_col, true);

    /* If one forward is free, go there */
    newPos = MakePosition(results->data[results->count - 1].pos.column + coef_col, results->data[results->count - 1].pos.row + coef_row);
    if (CheckOutOfBounds(board, newPos.row, newPos.column) && board->data[newPos.row][newPos.column] == ' ')
        CallQueenMovesJumped(board, results, newPos, coef_row, coef_col, false);
    
}

/* Logic of all possible queen moves before the first jump */
void QueenMovesNonJumped(Vec2d* board, Vec* results, int coef_row, int coef_col)
{
    Position newPos = {results->data[0].pos.column + coef_col, results->data[0].pos.row + coef_row};
    while (CheckOutOfBounds(board, newPos.row, newPos.column))
    {
        if (board->data[newPos.row][newPos.column] == ' ')
        {
            printf("%c%d -> %c%d\n", IndexIntToChar(results->data[0].pos.column), results->data[0].pos.row + 1, IndexIntToChar(newPos.column), newPos.row + 1);
            programmer_sin++;
            newPos.column += coef_col; newPos.row += coef_row;
            continue;
        }
        else if (board->data[newPos.row][newPos.column] == 'b' || board->data[newPos.row][newPos.column] == 'B')
        {
            if (CheckOutOfBounds(board, newPos.row + coef_row, newPos.column + coef_col) && board->data[newPos.row + coef_row][newPos.column + coef_col] == ' ')
            {
                Cell pass = MakeCell(MakePosition(newPos.column + coef_col, newPos.row + coef_row), ' ');
                Vec_pushback(results, &pass);
                results->jumps++;
                board->data[newPos.row][newPos.column] = ' ';
                QueenMovesJumped(board, results, coef_row, coef_col);
                Vec_popback(results);
                results->jumps--;
                board->data[newPos.row][newPos.column] = 'b';
            }
            return;
        }
        else
            return;
    }
}

void QueenMoves(Vec2d* board, Vec* results)
{
    /* DOWN_RIGHT */
    QueenMovesNonJumped(board, results, +1, +1);
    /* UP_RIGHT */
    QueenMovesNonJumped(board, results, -1, +1);
    /* DOWN_LEFT */
    QueenMovesNonJumped(board, results, +1, -1);
    /* UP_LEFT */
    QueenMovesNonJumped(board, results, -1, -1);
}

/* 
    Iterating through position (sorted w/W -> b/B),
    calling corresponding functions based on Cell type,
    when reaching the last Cell or encounter B (all white Cells have already passed) -> program ends,
    frees memory
*/
void CalculateMoves(Vec2d* board, Vec* positions)
{
    Vec_sort(positions);
    Vec results; Vec_init(&results);

    for (size_t i = 0; i < positions->count; i++)
    {
        Vec_reset(&results);
        Vec_pushback(&results, &positions->data[i]);
        
        if (positions->data[i].type == 'w')
            BasicMoves(board, &results);
        else if (positions->data[i].type == 'W')
        {
            board->data[positions->data[i].pos.row][positions->data[i].pos.column] = ' ';
            QueenMoves(board, &results);
            board->data[positions->data[i].pos.row][positions->data[i].pos.column] = 'W';
        }
        else
            break;
    }

    /* Prints message -> number of possible moves */
    printf("Celkem ruznych tahu: %ld\n", programmer_sin);

    /* Frees all memory */
    Vec_free(&results);
    Vec_free(positions);
    Vec2d_free(board);
}

/* 
    Called from main,
    declares: 
            Vec2d board     -> stores n x n gameboard
            Vec positions   -> stores info about all the cells i dont have to iterate through the board to find white Cells
    calls coresponding functions to be executed,
    returns true/false based on success/failure
*/
bool Program()
{
    Vec2d board;
    Vec positions;

    if(!HandleInput(&board, &positions))
    {
        printf("Nespravny vstup.\n");
        return false;
    }

    /* Calls function to calculate all the possible moves */
    CalculateMoves(&board, &positions);
    return true;
}

int main()
{
    if (!Program())
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}