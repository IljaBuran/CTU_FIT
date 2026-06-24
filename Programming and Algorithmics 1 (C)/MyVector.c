#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdarg.h>

#define RESIZE_COEFICIENT 2

/* fill function */
#define MAX_ARGUMENTS 10
#define PRIME prime
#define NOT_PRIME not_prime
#define DIVISIBLE_BY(by) divisible_by, &(int){by} /* because of this, program works ONLY on gcc compiler */
#define NOT_DIVISIBLE_BY(by) not_divisible_by, &(int){by}
#define EVEN DIVISIBLE_BY(2)
#define ODD NOT_DIVISIBLE_BY(2)
#define PERFECT_SQUARE perfect_square
#define PALINDROM palindrom


/* sorting */
#define ASCENDING integer_comparison_asc
#define DESCENDING integer_comparison_desc

/* file system */
#define APPEND "a"
#define WRITE "w"
#define READ "r"


typedef struct
{
    int* data;
    size_t count;
    size_t capacity;
}Vector;

///////////////////////////////////////////////////////
/* Custom functions to be passed to vector functions */
///////////////////////////////////////////////////////

/* figures out whether value passed is prime number */
/* parameter unused is useless in terms of function itself, though it's needed for compatibilty with vector_fill function */
bool prime(const int* val, void* unused)
{
    if (*val < 2)
        return false;
    if (*val == 2)
        return true;
    if (*val % 2 == 0)
        return false;
    for (int i = 3; i <= sqrt(*val); i += 2)
        if (*val % i == 0)
            return false;
    return true;
}

/* figures out whether value passed is not prime number */
bool not_prime(const int* val, void* unused)
{
    return !prime(val, NULL);
}

/* figures out whether value passed is divisible by passed number */
bool divisible_by(const int* val, void* by)
{
    if (*(int*)by == 0)
    {
        printf("Division by 0 in divisible_by function\n.");
        return false;
    }
    return (*val % *(int*)by == 0);
}

/* figures out whether value passed is not divisible by passed number */
bool not_divisible_by(const int* val, void* by)
{
    return !divisible_by(val, by);
}

bool contains_number(const int* val, void* num)
{
    int number = *val;
    while (number !=  0)
    {
        if (number % 10 == *(int*)num)
            return true;
        number /= 10;
    }
    return false;
}

/* figures out whether value contains a number in it */
bool palindrom(const int* val, void* unused)
{
    int original = *val, reversed = 0, remainder;
    if (original < 0)
        return false;
    
    int temp = original;
    
    while (temp != 0)
    {
        remainder = temp % 10;
        reversed = reversed * 10 + remainder;
        temp /= 10;
    }
    return original == reversed;
}

bool perfect_square(const int* val, void* unused)
{
    if (*val < 0)
        return false;
    
    int root = (int)sqrt((double)*val);
    return root * root == *val;
}

/* ascending sorting */
int integer_comparison_asc(const void* a, const void* b)
{
    return *(int*)a - *(int*)b;
}

/* descending sorting */
int integer_comparison_desc(const void* a, const void* b)
{
    return *(int*)b - *(int*)a;
}

/* Initialize vector, allocates memory for array */
/* Only called after declaring Vector, or after freeing */
bool vector_init(Vector* vec)
{
    if (vec->data != NULL)
    {
        printf("vector_init: calling with invalid parameter: initialized vector.\n");
        return false;
    }
    
    vec->capacity = 5;
    vec->data = (int*)malloc(vec->capacity * sizeof(int));
    vec->count = 0;

    if (vec->data == NULL)
    {
        printf("vector_init: allocating memory failed.\n");
        return false;
    }

    return true;
}

/* checks whether resize is needed, if so -> doubles the capacity */
bool vector_resize(Vector* vec)
{
    if (vec->capacity == vec->count)
    {
        vec->capacity *= RESIZE_COEFICIENT;
        vec->data = (int*)realloc(vec->data, vec->capacity * sizeof(int));
            if (vec->data == NULL)
            {
                printf("vector_resize: eallocating memory failed.\n");
                return false;
            }
    }
    return true;
}

bool vector_shrink_to_fit(Vector* vec)
{
    if (vec->capacity > vec->count)
    {
        vec->capacity = vec->count;
        vec->data = (int*)realloc(vec->data, vec->capacity * sizeof(int));

        if (!vec->data)
        {
            printf("vector_shrink_to_fit: reallocating memory failed.\n");
            return false;
        }
    }
    return true;
}

/* adds element */
bool vector_pushback(Vector* vec, int val)
{
    if (!vector_resize(vec))
        return false;
    vec->data[vec->count++] = val;
    return true;
}

/* prints all elements */
void vector_print(Vector* vec)
{
    printf("[");
    for (size_t i = 0; i < vec->count; i++)
    {
        printf("%d", vec->data[i]);
        if (i != vec->count - 1)
            printf(", ");
    }
    printf("]\n");
}
/* Prints vector contents into file */
/* opt -> whet */
bool vector_print_file(Vector* vec, bool secure, char* file_name, char* opt)
{
    if (!vec->data)
    {
        printf("Function vector_print_file called for uninitialized vector\n");
        return false;
    }
    if (opt[0] != 'a' && opt[0] != 'w')
    {
        printf("Function vector_print_file invalid opt argument.\n");
        return false;
    }
    
    FILE* file;

    file = fopen(file_name, "r");
    if (file && secure)
    {
        printf("File exists, aborting before corupting file.\n");
        return false;
    }

    file = fopen(file_name, opt);

    if (file == NULL)
    {
        printf("Function vector_print_file could not create a file.\n");
        return false;
    }

    fprintf(file, "[\n");
    for (size_t i = 0; i < vec->count; i++)
    {
        fprintf(file, "%d", vec->data[i]);
        if (i != vec->count - 1)
            fprintf(file, ", \n");
    }
    fprintf(file, "\n]\n");

    fclose(file);

    return true;
}

/* Sorts array, depending on function passed */
void vector_sort(Vector* vec, int (*function)(const void*, const void*))
{
    qsort(vec->data, vec->count, sizeof(int), function);
}

/* Frees memory */
bool vector_free(Vector* vec)
{
    if (!vec->data)
    {
        printf("vector_free: function called for uninitialized vector\n");
        return false;
    }

    free(vec->data);
    vec->data = NULL;
    vec->count = 0;
    vec->capacity = 0;

    return true;
}

/* can be used only for existing */
/* deletes all elements, frees, and initializes the vector */
bool vector_reset(Vector* vec)
{
    if (!vector_free(vec) || !vector_init(vec))
        return false;
    
    return true;
}

/* returns true/false if val is found */
/* if found, assigns array position of element into index */
bool vector_search_linear(Vector* vec, int* index, int val)
{
    for (size_t i = 0; i < vec->count; i++)
        if (vec->data[i] == val)
        {
            if (index)
                *index = i;
            return true;
        }
    return false;
}

/* Attention: doesn't make sense calling on unsorted array */
/* Recommended to remove duplicates before applying this function */
bool vector_search_binary(Vector* vec, int* index, int val)
{
    size_t low = 0, high = vec->count - 1;
    
    while (low <= high)
    {
        size_t mid = low + (high-low) / 2;

        if (vec->data[mid] == val)
        {
            if (index)
                *index = mid;
            return true;
        }
        else if (vec->data[mid] < val)
            low = mid + 1;
        else
        {   
            if (mid == 0)
                break;
            high = mid - 1;
        }
    }
    return false;
}

/* continues, doesn't overwrite already added elements from beginning */
/* basically push_back with multiple elements */
/* argument_count - number of condition functions */
/* ... -> condition functions (const int*, void*), functions need to stick with interface */

void vector_fill(Vector* vec, int from, int to, size_t argument_count, ...)
{
    /* error handling */
    if (vec->data == NULL) 
    { 
        printf("Uninitialized vector passed into vector_fill function.\n"); 
        return; 
    }
    if (argument_count > MAX_ARGUMENTS) 
    { 
        printf("Argument count overreached.\n"); 
        return; 
    }

    va_list args;
    bool (*conditions[MAX_ARGUMENTS])(const int*, void* context)= {0};
    void* contexts[MAX_ARGUMENTS] = {0};
    
    /* if there are no arguments -> adds all numbers */
    if (argument_count == 0)
    {
        for ( ; from <= to; from++)
            vector_pushback(vec, from);
        return;
    }
    
    /* filling conditions and contexts with arguments */
    va_start(args, argument_count);
    for (size_t i = 0; i < argument_count; i++)
    {
        conditions[i] = va_arg(args, bool (*)(const int*, void* context));
        contexts[i] = va_arg(args, void*);
    }
    va_end(args);


    /* check whether current number passes thorugh condition(s) */
    for ( ; from <= to; from++)
    {
        bool add = true;
        for (size_t i = 0; i < argument_count; i++)
            if (!conditions[i](&from, contexts[i]))
            {
                add = false; 
                break;
            }
        if (add)
            vector_pushback(vec, from);
    }
}

bool vector_fill_from_file(Vector* vec, char* file_name, size_t argument_count, ...)
{
    FILE* file = fopen(file_name, "r");

    if (!file)
    {
        printf("Couldn't open file\n");
        return false;
    }

    while(true)
    {
        int num;
        int n = fscanf(file, "%d", &num);
        if (n == EOF)
            break;
        else if (n == 1)
            vector_pushback(vec, num);
        else
            fscanf(file, "%*s");
    }

    fclose(file);
    return true;
}

/* naive */
void vector_remove_duplicates(Vector* vec, bool sorted)
{
    if (vec->count < 2)
        return;

    Vector res = {0};
    vector_init(&res);
    
    if (sorted)
    {
        vector_pushback(&res, vec->data[0]);
        for (size_t i = 1; i < vec->count; i++)
        {
            if (vec->data[i] == res.data[res.count - 1])
                continue;
            vector_pushback(&res, vec->data[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < vec->count; i++)
        {
            if (vector_search_linear(&res, NULL, vec->data[i]))
                continue;
            vector_pushback(&res, vec->data[i]);
        }
    }

    vector_free(vec);
    *vec = res;
}

bool vector_remove_element(Vector* vec, size_t index)
{
    if (vec->count == 0)
    {
        printf("vector_remove_element: deleting from empty vector.\n");
        return false;
    }
    
    if (index >= vec->count)
    {
        printf("vector_remove_element: out of range index.\n");
        return false;
    }
    for (size_t i = index; i < vec->count - 1; i++)
        vec->data[i] = vec->data[i + 1];
    
    vec->count--;
    return true;
}

bool vector_pop(Vector* vec)
{
    if (vec->count == 0)
    {
        printf("vector_pop: deleting from empty vector.\n");
        return false;
    }

    vec->count--;
    return true;
}

int main()
{
    Vector vec = {0};
    if (!vector_init(&vec))
        return -1;

    vector_fill_from_file(&vec, "text.txt", 0);

    vector_print(&vec);

    vector_remove_element(&vec, 1);

    vector_print(&vec);

    vector_free(&vec);

    return 0;
}