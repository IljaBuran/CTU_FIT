#include <stdio.h>
/* Hopefully PT doesn't mind :) */
#include <ctype.h> /* needed isspace function */
#include <string.h> /* needed strlen function */
int main()
{
    /* Variables */
    int in; char c[50] = {0};
    
    /* Prints message + scanfs input */
    printf("ml' nob:\n");
    if(scanf("%d", &in) != 1)
    {
        printf("Neh mi'\n"); return 1;
    }
    
    /* Make sure there is no non-whitespace character */
    fgets(c, sizeof(c), stdin); 
    for(size_t i = 0; i < strlen(c); i++)
        if(!isspace(c[i]))
        {
            printf("bIjatlh 'e' yImev\n"); return 1;
        }
    
    /* Check if input digit is inside of valid range */
    if(in < 0 || in > 8)
    {
        printf("Qih mi' %d\n", in); return 1; 
    }
    
    /* Input is correct */
    printf("Qapla'\n");
    if(in == 0) { printf("noH QapmeH wo' Qaw'lu'chugh yay chavbe'lu' 'ej wo' choqmeH may' DoHlu'chugh lujbe'lu'.\n"); }
    else if(in == 1) { printf("bortaS bIr jablu'DI' reH QaQqu' nay'.\n"); }
    else if(in == 2) { printf("Qu' buSHa'chugh SuvwI', batlhHa' vangchugh, qoj matlhHa'chugh, pagh ghaH SuvwI''e'.\n"); }
    else if(in == 3) { printf("bISeH'eghlaH'be'chugh latlh Dara'laH'be'.\n"); }
    else if(in == 4) { printf("qaStaHvIS wa' ram loS SaD Hugh SIjlaH qetbogh loD.\n"); }
    else if(in == 5) { printf("Suvlu'taHvIS yapbe' HoS neH.\n"); }
    else if(in == 6) { printf("Ha'DIbaH DaSop 'e' DaHechbe'chugh yIHoHQo'.\n"); }
    else if(in == 7) { printf("Heghlu'meH QaQ jajvam.\n"); }
    else if(in == 8) { printf("leghlaHchu'be'chugh mIn lo'laHbe' taj jej.\n"); }

    return 0;
}