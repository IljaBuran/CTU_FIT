#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#define INIT_CAPACITY 10
#define RESIZE_COEFICIENT 2

typedef int Stock;

/* Function called when invalid input from user is entered, when called program is aborted */
inline bool Invalid()
{
    printf("Nespravny vstup.\n");
    return false;
}

/* Struct where stocks data are stored */
/* data -> dynamically allocated array with stock prices */
/* count -> keeps track of how many elements are stored in data */
/* capacity -> stores a value of maximum elements that can be store in data */
typedef struct
{
    Stock* data;
    size_t count;
    size_t capacity;
}Stocks;

/* Struct to keep info about min/max in FindMinMax function together */
typedef struct
{
    size_t from;
    size_t to;
    Stock value;
}Found;

/* Initializes/allocates array */
bool Stocks_Init(Stocks* stocks)
{
    stocks->capacity = INIT_CAPACITY;
    stocks->data = (Stock*)malloc(stocks->capacity * sizeof(Stock));
    stocks->count = 0;

    if (!stocks->data)
        return false;

    return true;
}

/* Deallocates stocks array */
void Stocks_Free(Stocks* stocks)
{
    if (!stocks)
        return;
    free(stocks->data);
    stocks->data = NULL;
    stocks->count = 0;
    stocks->capacity = 0;
}

/* Increases size of data[] */
bool Stocks_Resize(Stocks* stocks)
{
    /* If number of elements reaches it's maximum (capacity), array increases it's volume */
    if (stocks->count == stocks->capacity)
    {
        stocks->capacity *= RESIZE_COEFICIENT;
        stocks->data = (Stock*)realloc(stocks->data, stocks->capacity * sizeof(Stock));
        
        if (!stocks->data)
            return false;
    }
    return true;
}

/* Assigns val into array */
bool Stocks_Pushback(Stocks* stocks, Stock val)
{
    /* Check whether data[] needs resizing */
    if (!Stocks_Resize(stocks))
        return false;

    /* assigns val into count-position of data[] and increments count */
    stocks->data[stocks->count++] = val;

    return true;
}

/* Finds and prints min. and max. profit (loss) */
void FindMinMax(Stocks* stocks, size_t from, size_t to)
{
    /* If parameters if == to, prints obvious result and returns */
    if (from == to)
    {
        printf("Nejvyssi zisk: N/A\nNejvyssi ztrata: N/A\n");
        return;
    }

    /* Found min, max will be updated with results */
    Found min = {0}, max = {0};
    /* maxPrice and minPrice keep track of all time maximum/minimum prices */
    int maxPrice = stocks->data[from];
    int minPrice = stocks->data[from];
    /* minIndex and maxIndex keep track index of maximum/minimum prices */
    size_t minIndex = from;
    size_t maxIndex = from;

    /* Iterating through array */
    for (size_t i = from + 1; i <= to; i++)
    {
        /* Profit */
        int diff = stocks->data[i] - minPrice;
        if (stocks->data[i] < minPrice)
        {
            minPrice = stocks->data[i];
            minIndex = i;
        }
        if (diff > max.value)
        {
            max.value = diff;
            max.from = minIndex;
            max.to = i;
        }

        /* Loss */
        diff = stocks->data[i] - maxPrice;
        if (stocks->data[i] > maxPrice)
        {
            maxPrice = stocks->data[i];
            maxIndex = i;
        }
        if (diff < min.value)
        {
            min.value = diff;
            min.from = maxIndex;
            min.to = i;
        }
    }
    
    /* Prints result */
    printf("Nejvyssi zisk: ");
    max.value == 0 ? printf("N/A\n") : printf("%d (%ld - %ld)\n", max.value, max.from, max.to);
    printf("Nejvyssi ztrata: ");
    min.value == 0 ? printf("N/A\n") : printf("%d (%ld - %ld)\n", (int)fabs(min.value), min.from, min.to);
}

/* Checks validity of 'from' and 'to' input */
bool ValidFromTo(int from, int to, size_t count)
{
    if (from < 0 || to < 0 || from > to || to >= (int)count)
        return false;
    return true;
}

/* Loops throught stdin if there is non-whitespace symbol */
bool CheckBuffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) 
        if (!isspace(ch)) 
            return false; 
    return true;
}

/* Main logic of handling input from user, calls other functions based on input */
bool HandleInput(Stocks* stocks)
{
    while(true)
    {
        /* Scanfs operation symbol */
        char opt;
        int i = scanf(" %c", &opt);

        /* If end of file -> reading SUCCESS */
        if (i == EOF)
            break;

        /* Checks whether input was valid */
        if (i != 1 || (opt != '+' && opt != '?'))
            return Invalid();

        if (opt == '+') /* adding stock price */
        {
            Stock stockPrice;
            /* scanfs stockPrice and checks input validity */
            if (scanf("%d", &stockPrice) != 1 || stockPrice < 0 || !CheckBuffer())
                return Invalid();
            Stocks_Pushback(stocks, stockPrice);
        }
        else /* inquiry */
        {
            int from, to;
            /* scanfs from, to and checks input validity */
            if (scanf("%d%d", &from, &to) != 2 || !ValidFromTo(from, to, stocks->count) || !CheckBuffer())
                return Invalid();
            FindMinMax(stocks, (size_t)from, (size_t)to);
        }
    }
    return true;
}

/* Main block of program */
bool Program()
{
    /* Declare stocks and initialize/allocate it */
    Stocks stocks = {0}; 
    Stocks_Init(&stocks);

    /* Print message to user */
    printf("Ceny, hledani:\n");

    /* Call HandleInput, if false -> program ends with fail */
    if (!HandleInput(&stocks))
    {
        Stocks_Free(&stocks);
        return false;
    }
    
    Stocks_Free(&stocks);
    return true;
}

int main()
{
    if (!Program())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}