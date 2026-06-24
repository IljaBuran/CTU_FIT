#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define FAIL -2

typedef struct
{
    int lower, upper;
    char sign, c1, c2, c3;
}Prompt;

bool IsPromptValid(Prompt p) /* Check if prompt values are valid */
{
    if (p.c1 != '<' || p.c2 != ';' || p.c3 != '>' || p.lower > p.upper || p.lower < 1 || (p.sign != '#' && p.sign != '?')) 
        return false; 
    return true;
}

int PromptInput(Prompt* p) /* scanfs prompt into p, checks validity and returns value from scanf or fail value*/
{
    int i = scanf(" %c %c %d %c %d %c", &p->sign, &p->c1, &p->lower, &p->c2, &p->upper, &p->c3);
    if(!IsPromptValid(*p)) return FAIL;
    return i;
}

void FindRightTriangles(Prompt p) /* Finds right triangles based on prompt p (upper, lower interval) and prints result */
{
    int count = 0; /* Count of right angled triangles found */
    /* Algorithm */
    for (int c = p.lower; c <= p.upper ; c++)
    {
        for (int a = p.lower; a <= p.upper && a < c; a++)
        {
            int b_ = c * c - a * a; /* b_ == b*b == c*c - a*a  */
            
            if (b_ <= 0) continue; /* is b valid? if not, skip current iteration */
            int b = (int)sqrt(b_); /* square root of b and casting to int -> sqrt() returns double! */
            if (b * b != b_ || (b < a || b > p.upper)) continue; /* check if type casted b * b is equal to  b_? check whether a < b < p.upper? if not, skip current iteration*/
            if (p.sign == '?') /* if sign == ?, print values */
                printf("* %d %d %d\n", a, (int)b, c); 
            count++; /* increment result */
        }
    }
    printf("Celkem: %d\n", count); /* print result */
}

int main()
{
    printf("Problemy:\n");

    while(true)
    {
        Prompt p;
        int out = PromptInput(&p);  /* scanfs prompt into p + writes value into out */
        if (out == EOF) break; /* output value from scanf == EOF -> loop ends */
        if (out != 6) { printf("Nespravny vstup.\n"); return 1; } /* incorrect number of readings AND/OR invalid values -> program fails */

        FindRightTriangles(p);
    }
    return 0;
}