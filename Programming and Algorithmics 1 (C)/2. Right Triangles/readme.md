
Úkolem je realizovat program, který bude počítat pravoúhlé trojúhelníky.

Uvažujeme pravoúhlé trojúhelníky, které mají všechny strany celočíselné délky. Dále uvažujeme uzavřený interval celých čísel  <a;b>. Chceme najít a spočítat/vypsat všechny pravoúhlé trojúhelníky, které mají celočíselnou délku stran a které mají všechny tři strany v zadaném intervalu hodnot. Například pro interval hodnot  <4;14>  nalezneme následující dva trojúhelníky:

5 12 13
6 8 10

Do odpovědi nepatří trojúhelníky  3 4 5  ani  9 12 15, protože některé jejich strany jsou mimo zadaný interval.

Při počítání trojúhelníků uvažujeme pouze unikátní tvary trojúhelníku. Tedy například trojúhelníky  5 12 13  a  6 8 10  jsou dva různé trojúhelníky, které oba zahrneme do výsledku. Naproti tomu trojúhelníky  5 12 13  a  12 5 13  jsou shodné, tedy do výsledku zahrneme pouze jeden z nich.

Vstupem programu je zadávání problémů k vyřešení. Program zadané problémy zpracovává a zobrazuje jejich řešení. Zpracování vstupu končí v okamžiku, kdy je zpracován poslední zadaný problém (je dosaženo EOF na standardním vstupu). Vstupní problémy jsou dvojího druhu:

-   vypsání všech unikátních pravoúhlých trojúhelníků s celočíselnými délkami stran ze zadaného intervalu. Problém je zadán znakem otazník a mezemi intervalu:
    
        ? <lo;hi>
        
    
    kde  lo  a  hi  jsou celá čísla udávající meze intervalu. Odpovědí programu je seznam nalezených trojúhelníků (délek jejich stran), každý trojúhelník na jeden řádek (viz ukázka). Za koncem výpisu je zobrazen počet nalezených trojúhelníků,
-   spočítání unikátních pravoúhlých trojúhelníků s celočíselnými délkami stran ze zadaného intervalu. Problém je zadán znakem hash a mezemi intervalu:
    
        # <lo;hi>
        
    
    kde  lo  a  hi  jsou celá čísla udávající meze intervalu. Odpovědí programu je počet nalezených trojúhelníků.

Pokud vstup není platný, program tuto situaci detekuje, vypíše chybové hlášení a ukončí se. Formát chybového hlášení je opět uveden v ukázkách níže. Za chybu je považováno:

-   nerozpoznaný problém (nezačíná znakem ? ani #),
-   nesprávné zadání mezí intervalu (meze musí být celá kladná čísla, dolní mez musí být menší nebo rovná horní mezi),
-   chybějící oddělovače (menšítko, většítko, středník).

**Ukázka práce programu:**  

----------

**Problemy:**
? <10;30>
*** 10 24 26**
*** 12 16 20**
*** 15 20 25**
*** 18 24 30**
*** 20 21 29**
**Celkem: 5**
? <8;60>
*** 8 15 17**
*** 9 12 15**
*** 9 40 41**
*** 10 24 26**
*** 12 16 20**
*** 12 35 37**
*** 14 48 50**
*** 15 20 25**
*** 15 36 39**
*** 16 30 34**
*** 18 24 30**
*** 20 21 29**
*** 20 48 52**
*** 21 28 35**
*** 24 32 40**
*** 24 45 51**
*** 27 36 45**
*** 28 45 53**
*** 30 40 50**
*** 33 44 55**
*** 36 48 60**
*** 40 42 58**
**Celkem: 22**

<8;60>

**Celkem: 22**

<7;120>

**Celkem: 63**

----------

**Problemy:**
?<5;15>
*** 5 12 13**
*** 6 8 10**
*** 9 12 15**
**Celkem: 3**

< 3 ; 45 >

**Celkem: 18**
#<1;200>
**Celkem: 127**

<10;10>

**Celkem: 0**

----------

**Problemy:**
* <3;9>
**Nespravny vstup.**

----------

**Problemy:**

<12;11>

**Nespravny vstup.**

----------

**Problemy:**
? 10;20
**Nespravny vstup.**