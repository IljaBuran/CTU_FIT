#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

typedef struct /* Used for both, points and vectors */
{
    double x, y;
}Point;

bool PointInput(Point* p) /* Scanfs data + checks if they're valid */
{
    char c1, c2, c3;
    if(scanf(" %c %lf %c %lf %c", &c1, &p->x, &c2, &p->y, &c3) != 5)
        return false;
    else if(c1 != '[' || c2 != ',' || c3 != ']')
        return false;
    return true;
}

bool IsValid(Point a, Point b, Point c)
{
    /* Check if there are identical points */
    if( (a.x == b.x && a.y == b.y) || 
        (a.x == c.x && a.y == c.y) ||
        (b.x == c.x && b.y == c.y))
        return false;
    
    /* Check if slopes are valid */
    double  slopeAB = (b.y - a.y) / (b.x - a.x), 
            slopeBC = (c.y - b.y) / (c.x - b.x);
    if(fabs(slopeAB - slopeBC) < 1000 * DBL_EPSILON * slopeAB)
        return false;

    return true;
}

double DotProduct(Point p1, Point p2) { return (p1.x * p2.x) + (p1.y * p2.y); } /* Linear Algebra Flashback */

double Magnitude(Point p) { return sqrt(pow(p.x, 2) + pow(p.y, 2)); } /* Returns magnitude of a vector */

bool IsRightAngle(Point p1, Point p2) /* It does the obvious */
{
    return fabs(DotProduct(p1, p2)) < 1000 * DBL_EPSILON * Magnitude(p1) * Magnitude(p2);
}

const char* DetermineType(Point p1, Point p2) /* Calculates and returns type */
{
    double magnitudeDiff = fabs(Magnitude(p1) - Magnitude(p2));
    if(magnitudeDiff <= 1000 * DBL_EPSILON * Magnitude(p2))
    {
        if(IsRightAngle(p1, p2)) 
            return "ctverec\n";
        else 
            return "kosoctverec\n";  
    }
    else 
        if(IsRightAngle(p1, p2)) 
            return "obdelnik\n";
        else 
            return "rovnobeznik\n"; 
}

int main()
{
    /* Variables */
    Point A, B, C;

    /* Input of A, B, C */
    printf("Bod A:\n");
    if (!PointInput(&A)) { printf("Nespravny vstup.\n"); return 1; } /* Point A */
    printf("Bod B:\n");
    if (!PointInput(&B)) { printf("Nespravny vstup.\n"); return 1; } /* Point B */
    printf("Bod C:\n");
    if (!PointInput(&C)) { printf("Nespravny vstup.\n"); return 1; } /* Point C */

    /* Check points validity */
    if(!IsValid(A, B, C)) { printf("Rovnobezniky nelze sestrojit.\n"); return 1; }

    /* Calculating A', B', C' */
    /* A' == A_, ... */
    Point tempP1, tempP2, A_, B_, C_;
    
    /* Point A' */ 
    tempP1.x = C.x - A.x; tempP1.y = C.y - A.y;
    A_.x = B.x + tempP1.x; A_.y = B.y + tempP1.y;
    tempP1.x = B.x - A.x; tempP1.y = B.y - A.y;
    tempP2.x = C.x - A.x; tempP2.y = C.y - A.y;
    printf("A': [%.10g,%.10g], %s", A_.x, A_.y, DetermineType(tempP1, tempP2));
    
    /* Point B' */
    tempP1.x = A.x - B.x; tempP1.y = A.y - B.y;
    B_.x = C.x + tempP1.x; B_.y = C.y + tempP1.y;
    tempP1.x = B.x - C.x; tempP1.y = B.y - C.y;
    tempP2.x = B.x - A.x; tempP2.y = B.y - A.y;
    printf("B': [%.10g,%.10g], %s", B_.x, B_.y, DetermineType(tempP1, tempP2));
    
    /* Point C' */
    tempP1.x = A.x - C.x; tempP1.y = A.y - C.y;
    C_.x = B.x + tempP1.x; C_.y = B.y + tempP1.y;
    tempP1.x = C.x - A.x; tempP1.y = C.y - A.y;
    tempP2.x = C.x - B.x; tempP2.y = C.y - B.y;
    printf("C': [%.10g,%.10g], %s", C_.x, C_.y, DetermineType(tempP1, tempP2));

    return 0;
}