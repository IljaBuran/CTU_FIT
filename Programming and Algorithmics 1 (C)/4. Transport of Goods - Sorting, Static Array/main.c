#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* limit for transportations */
#define MAX_TRANSPORTS 100000

/* To keep the code tidy, isn't needed, but somewhere in the code passing all the values to check validity */
typedef struct 
{
    char c0, /*   [   */
         c1, /*   -   */
         c2, /*   ,   */
         c3, /*   ,   */
         c4, /*   ]   */
         c5; /* , or } */
}PromptCharacters;

/* To keep the code tidy and easier to work with */
typedef struct 
{
    int from, 
        to,
        capacity,   /* of pieces of goods */
        price;      /* daily */
}Transportation;

/* To keep the code tidy and easier to work with + TransportationArray.count keeps track of how many elements does arr hold */
typedef struct
{
    Transportation arr[MAX_TRANSPORTS];
    int count;
}TransportationArray;

typedef struct
{
    int from;
    long long quantity;
}Inquiry;

bool InputValidity(Transportation* t, PromptCharacters* c)
{
    /* Check if prompt characters are valid */
    if (c->c0 != '[' || c->c1 != '-' || c->c2 != ',' || c->c3 != ',' || c->c4 != ']')
        return false;
    /* Check if values are valid */
    else if (t->from > t->to || t->from < 0 || t->to < 0 || t->capacity <= 0 || t->price <= 0)
        return false;
    return true;
}

bool PromptInput(Transportation* t)
{
    PromptCharacters c;
    if (scanf(" %c %d %c %d %c %d %c %d %c", &c.c0, &t->from, &c.c1, &t->to, &c.c2, &t->capacity, &c.c3, &t->price, &c.c4) != 9 || !InputValidity(t, &c))
    {
        return false;
    }
    return true;
}

bool InquiryValidity(Inquiry* inq) { return inq->from >= 0 && inq->quantity > 0; }

int InquiryInput(Inquiry* inq)
{
    return scanf("%d%lld", &inq->from, &inq->quantity);
}

/* function for qsort */
int CompareByTo(const void* a, const void* b) 
{
    return ((Transportation*)a)->to - ((Transportation*)b)->to;
}

/* Binary Search for index */
int FindIndex(TransportationArray* ta, int from) 
{
    int low = 0;
    int high = ta->count;

    int result = -1;
    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        if (ta->arr[mid].to >= from)
        {
            result = mid;
            high = mid - 1;
        }
        else
            low = mid + 1;
    }
    return result;
}

/* sort Transportation array using qsort */
void SortArrayByTo(TransportationArray* ta)
{
    qsort(ta->arr, ta->count + 1, sizeof(Transportation), CompareByTo);
}

bool PhaseOne(TransportationArray* ta)
{
    /* Prints message to user */
    printf("Moznosti dopravy:\n");

    /* reads and checks if the first sign '{' is valid -> if not, program fails */
    char c1;
    if (scanf(" %c", &c1) != 1 || c1 != '{') { printf("Nespravny vstup.\n"); return false; } 

    /* main loop of PhaseOne */
    /* reads valid inputs until inside brackets {} */
    /* fails if current Transportationg input is not valid or exceeds input limit MAX_TRANSPORTS */
    for ( ta->count = 0; ta->count <= MAX_TRANSPORTS; ta->count++)
    {
        /* See if we exceeded limit of inputs -> if so, program fails */
        if (ta->count == MAX_TRANSPORTS){ printf("Nespravny vstup.\n"); return false; }
        
        /* declaring t -> there will be assigned new values from input */
        Transportation t;

        /* reads input and checks validity of input -> if fails, program fails */
        if (!PromptInput(&t)){ printf("Nespravny vstup.\n"); return false; }
        /* assign t value to array */
        ta->arr[ta->count] = t;
        /* read char to see, if there are going to be more inputs ',' or it's last one '}' */
        if (scanf(" %c", &c1) != 1 || (c1 != '}' && c1 != ',')) { printf("Nespravny vstup.\n"); return false; } 
        /* if reads '}' == end of transportation input */
        if (c1 == '}') break; 
    }
    SortArrayByTo(ta);
    return true;
}

bool PhaseTwo(TransportationArray* ta)
{
    printf("Naklad:\n");

    while(true)
    {
        Inquiry inq;
        int scanfVal = InquiryInput(&inq);
        if (scanfVal == EOF) return true;
        if (scanfVal != 2 || !InquiryValidity(&inq)) { printf("Nespravny vstup.\n"); return false; }

        long long currentPrice = 0;
        long long quantityLeft = inq.quantity;
        bool possible;
        int max = ta->arr[ta->count].to;
        if (inq.from > max) 
        {
            printf("Prilis velky naklad, nelze odvezt.\n");
            continue;
        }
        int index = FindIndex(ta, inq.from);

        while (true)
        {
            possible = false;
            for ( ; index <= ta->count; index++)
            {
                //printf("%d <= %d && %d >= %d\n", ta->arr[index].from, inq.from, ta->arr[index].to,inq.from);
                if ( ta->arr[index].from <= inq.from && ta->arr[index].to >= inq.from )
                {
                    possible = true;
                    currentPrice += ta->arr[index].price;
                    quantityLeft -= ta->arr[index].capacity;
                }
                else if (ta->arr[index].from > inq.from)
                {
                    possible = true;
                    //printf("Possible.\n");
                }
            }
            if (quantityLeft <= 0)
                break;
            if (!possible)
            {
                //printf("Impossible");
                break;
            }
            inq.from++;
            index = FindIndex(ta, inq.from);
            if (index == -1)
            {
                possible = false;
                break;
            }
        }

        if (possible)
            printf("Konec: %d, cena: %lld\n", inq.from, currentPrice);
        else
            printf("Prilis velky naklad, nelze odvezt.\n");
    }
}

#ifdef DEBUG
void PrintArray(TransportationArray* ta)
{
    for (int i = 0; i <= ta->count; i++)
        printf("%d. [ %d - %d, %d, %d ]\n", i, ta->arr[i].from, ta->arr[i].to, ta->arr[i].capacity, ta->arr[i].price);
}
#endif

int main()
{
    TransportationArray ta; 

    if (!PhaseOne(&ta)) return 1;
    
    #ifdef DEBUG
    PrintArray(&ta);
    #endif

    if (!PhaseTwo(&ta)) return 1;

    #ifdef DEBUG
    printf("Success.\n");
    #endif

    return 0;
}