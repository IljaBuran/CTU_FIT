#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

/* Definied constants */
#define INIT_SIZE 10
#define RESIZE_COEFICIENT 2

/* Cell struct */
typedef struct
{
    char letter;
    bool crossed;
}Cell;

/* Line struct -> array of Cells */
typedef struct
{
    Cell* data; /* array of cells */
    size_t count;
    size_t capacity;
}Line;

/* functions to control dynamically allocated Line structs */
void Line_Init(Line* line)
{
    line->capacity = INIT_SIZE;
    line->data = (Cell*) malloc(line->capacity * sizeof(Cell));
    line->count = 0;
}

void Line_Resize(Line* line)
{
    if (line->count == line->capacity)
    {
        line->capacity *= RESIZE_COEFICIENT;
        line->data = (Cell*)realloc(line->data, line->capacity * sizeof(Cell));
    }
}

void Line_ShrinkToFit(Line* line)
{
    line->capacity = line->count;
    line->data = (Cell*)realloc(line->data, line->capacity * sizeof(Cell));
}

void Line_Free(Line* line)
{
    free(line->data);
}

void Line_Pushback(Line* line, Cell c)
{
    Line_Resize(line);
    line->data[line->count++] = c;
}

void Line_Print(Line* line)
{
    for (size_t i = 0; i < line->count; i++)
        printf("%c", line->data[i].letter);
}

/* Matrix struct -> 2d array / array of Lines */
typedef struct
{
    Line* data; /* array of lines */
    size_t count;
    size_t capacity;
}Matrix;

/* functions to control dynamically allocated Matrix structs */
void Matrix_Init(Matrix* matrix)
{
    matrix->capacity = INIT_SIZE;
    matrix->data = (Line*)malloc(matrix->capacity * sizeof(Line));
    matrix->count = 0;
}

void Matrix_Resize(Matrix* matrix)
{
    if (matrix->capacity == matrix->count)
    {
        matrix->capacity *= RESIZE_COEFICIENT;
        matrix->data = (Line*)realloc(matrix->data, matrix->capacity * sizeof(Line));
    }
}

void Matrix_Free(Matrix* matrix)
{
    for (size_t i = 0; i < matrix->count; i++)
        Line_Free(&matrix->data[i]);

    free(matrix->data);
}

void Matrix_Pushback(Matrix* matrix, Line* line)
{
    Matrix_Resize(matrix);
    
    Line newLine;

    newLine.capacity = line->capacity;
    newLine.count = line->count;

    newLine.data = (Cell*)malloc(newLine.capacity * sizeof(Cell));

    memcpy(newLine.data, line->data, newLine.count * sizeof(Cell));

    matrix->data[matrix->count++] = newLine;
}

/* validity check of input */
bool IsCharacterValid(char c)
{
    return ((c > 96 && c < 123) || c == 46);
}

/* Naive algorithm to find words in matrix */
int TryAllDirections(Matrix* matrix, int col, int row, Line* word, bool cross)
{
    static const int directions[] = {-1, 0, 1};
    int wordLength = (int)word->count;
    int res = 0;
    for (int i = 0; i < 3 ; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if ((i == 1 && j == 1))
                continue;

            if (col + (wordLength - 1) * directions[i] < 0 || col + (wordLength - 1) * directions[i] >= (int)matrix->count)
                continue;

            if (row + (wordLength - 1) * directions[j] < 0 || row + (wordLength - 1) * directions[j] >= (int)matrix->data[0].count)
                continue;

            bool fail = false;
            for (int k = 0; k < wordLength; k++)
            {
                if (matrix->data[col + k * directions[i]].data[row + k * directions[j]].letter != word->data[k].letter)
                {
                    fail = true;
                    break;
                }
            }
            if (!fail)
            {
                res++;
                if (cross)
                for (int k = 0; k < wordLength; k++)
                    matrix->data[col + k * directions[i]].data[row + k * directions[j]].crossed = true;
            }            
        }
    }
    return res;
}

/* Search for first letter of word in Matrix and call TryAllDirections */
void SearchWords(Matrix* matrix, Line* word, bool cross)
{
    int res = 0;
    for (size_t i = 0; i < matrix->count; i++)
        for (size_t j = 0; j < matrix->data[i].count; j++)
            if (matrix->data[i].data[j].letter == word->data[0].letter)
                res += TryAllDirections(matrix, (int)i, (int)j, word, cross);
    
    Line_Print(word); 
    printf(": %dx\n", res);
}

/* Prints found result */
void PrintSolution(Matrix* matrix)
{
    int count = 60;
    printf("Tajenka:");
    for (size_t i = 0; i < matrix->count; i++)
    {
        for (size_t j = 0; j < matrix->data[i].count; j++)
        {
            if (!matrix->data[i].data[j].crossed && matrix->data[i].data[j].letter != '.')
            {
                if (count == 60)
                {    
                    count = 0;
                    printf("\n");
                }
                printf("%c", matrix->data[i].data[j].letter);
                count++;
            }
        }
    }
    printf("\n");
}

/* Input logic of reading lines for matrix */
bool ReadLines(Matrix* matrix)
{
    Line line;
    Line_Init(&line);
    size_t lineSize = (size_t) - 1;
    
    while (true)
    {
        line.count = 0;
        for (size_t i = 0; i <= lineSize; i++)
        {
            int c = getchar();
            if (i == lineSize && c != '\n')
            {
                Line_Free(&line);
                return false;
            }

            if (c == '\n')
            {
                if (i == 0)
                {
                    if (matrix->count == 0)
                    {
                        Line_Free(&line);
                        return false;
                    }
                    Line_Free(&line);
                    return true;
                }
                if (lineSize == (size_t) - 1)
                    lineSize = i;
                else
                {
                    if (i < lineSize)
                    {   
                        Line_Free(&line);
                        return false;
                    }
                }
                break;
            }
    
            if (!IsCharacterValid(c))
            {
                Line_Free(&line);
                return false;
            }

            Cell cl = {(char)c, false};
            Line_Pushback(&line, cl);
        }
        Matrix_Pushback(matrix, &line);
    }
}

/* Input logic of reading prompts */
bool ReadPrompt(Matrix* matrix)
{
    int c;
    Line line;
    Line_Init(&line);

    while (true)
    {
        line.count = 0;
        bool found = false;
        c = getchar();
        if (c == '-' || c == '#')
        {
            char opt = c;
            while(true)
            {
                c = getchar();
                if (isspace(c))
                {
                    if (!found)
                        continue;
                    else if (c == '\n')
                    {
                        if (line.count > 1)
                        {
                            if (opt == '-')
                                SearchWords(matrix, &line, true);
                            else
                                SearchWords(matrix, &line, false);
                            break;
                        }
                        Line_Free(&line);
                        return false;
                    }
                    else
                    {
                        Line_Free(&line);
                        return false;
                    }
                }
                else 
                {
                    found = true;
                    if (!IsCharacterValid(c))
                    {
                        Line_Free(&line);
                        return false;
                    }
                    Cell cl = {(char)c, false};
                    Line_Pushback(&line, cl);
                }
            }
        }
        else if (c == '?')
        {
            if ((c = getchar()) != '\n' && c != EOF)
            {
                Line_Free(&line);
                return false;
            }    
            PrintSolution(matrix);
        }
        else if (c == EOF)
        {
            Line_Free(&line);
            return true;
        }
        else 
        {
            Line_Free(&line);
            return false;
        }
    }
}

bool HandleInput()
{
    Matrix matrix;
    Matrix_Init(&matrix);
    
    printf("Osmismerka:\n");

    if (!ReadLines(&matrix))
    {
        Matrix_Free(&matrix);
        return false;
    }

    if (!ReadPrompt(&matrix))
    {
        Matrix_Free(&matrix);
        return false;
    }

    Matrix_Free(&matrix);
    return true;
}


int main()
{
    if (!HandleInput())
    {
        printf("Nespravny vstup.\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}