
Úkolem je vytvořit program, který bude počítat možné tahy kamenů ve hře dáma.

Předpokládáme hru dáma se standardními pravidly. Pro zobecnění však uvažujeme čtvercovou hrací plochu o rozměru až 26 x 26 políček. Uvažujeme obyčejné kameny i dámy. Na vstupu programu je zadaná velikost plochy a rozmístění jednotlivých figurek. Program má za úkol určit možné tahy bílého. Tedy určit všechny různé platné tahy spolu s počtem zajatých kamenů černého.

Vstupem programu je nejprve velikost hrací plochy (celé číslo, max 26). Dále následuje seznam kamenů. Na jedné řádce je zadaný jeden kámen. Zadání má obecný tvar:

typ sloupec řádka

kde typ představuje typ kamene (w - bílý kámen, W - bílá dáma, b - černý kámen, B - černá dáma), sloupec je písmeno malé abecedy (a, b, ...) a řádka je celé číslo (1, 2, ...). Zadávání kamenů končí při dosažení EOF.

Výstupem programu je výpis možných tahů. Formát tahů je zřejmý z ukázky, za posloupností políček je uveden počet zajatých kamenů černého (pokud je nějaký černý kámen zajat). Poslední řádka výpisu udává počet možných různých tahů.

Pokud je vstup neplatný, program to musí detekovat a zobrazit chybové hlášení. Chybové hlášení zobrazujte na standardní výstup (ne na chybový výstup). Za chybu považujte:

-   velikost hrací plochy je nečíselná nebo mimo interval 3 až 26,
-   zadaný sloupec nebo řádek pozice kamene je mimo hrací plochu,
-   typ kamene není w, W, b ani B,
-   souřadnice není tmavé políčko (a1 je tmavé),
-   na jednom políčku byly zadané dva nebo více kamenů.

**Ukázka práce programu:**  

----------

**Velikost hraci plochy:**
10
**Pozice kamenu:**
w b4
w d4
w f4
b a5
b c5
B e5
b f6
w e3
w h4
**b4 -> d6 +1**
**d4 -> b6 +1**
**f4 -> g5**
**f4 -> d6 +1**
**h4 -> i5**
**h4 -> g5**
**Celkem ruznych tahu: 6**

----------

**Velikost hraci plochy:**
10
**Pozice kamenu:**
w a1
w g1
w i1
w h2
w j2
w g3
w i3
b b2
b d4
b h4
b d6
B f6
b d8
b f8
b h8
b i9
b b10
**a1 -> c3 +1**
**a1 -> c3 -> e5 +2**
**a1 -> c3 -> e5 -> g7 +3**
**a1 -> c3 -> e5 -> g7 -> e9 +4**
**a1 -> c3 -> e5 -> c7 +3**
**a1 -> c3 -> e5 -> c7 -> e9 +4**
**g1 -> f2**
**g3 -> i5 +1**
**g3 -> f4**
**i3 -> j4**
**i3 -> g5 +1**
**i3 -> g5 -> e7 +2**
**i3 -> g5 -> e7 -> c9 +3**
**i3 -> g5 -> e7 -> g9 +3**
**Celkem ruznych tahu: 14**

----------

**Velikost hraci plochy:**
10
**Pozice kamenu:**
W a1
W g1
W i1
W h2
W j2
W g3
W i3
b b2
b d4
b h4
b d6
B f6
b d8
b f8
b h8
b i9
b b10
**a1 -> c3 +1**
**a1 -> c3 -> e5 +2**
**a1 -> c3 -> e5 -> c7 +3**
**a1 -> c3 -> e5 -> c7 -> e9 +4**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 +5**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 -> e5 +6**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 -> d4 +6**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 -> c3 +6**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 -> b2 +6**
**a1 -> c3 -> e5 -> c7 -> e9 -> g7 -> a1 +6**
**a1 -> c3 -> e5 -> c7 -> e9 -> h6 +5**
**a1 -> c3 -> e5 -> c7 -> e9 -> i5 +5**
**a1 -> c3 -> e5 -> c7 -> e9 -> j4 +5**
**a1 -> c3 -> e5 -> c7 -> f10 +4**
**a1 -> c3 -> e5 -> c7 -> f10 -> i7 +5**
**a1 -> c3 -> e5 -> c7 -> f10 -> j6 +5**
**a1 -> c3 -> e5 -> b8 +3**
**a1 -> c3 -> e5 -> a9 +3**
**a1 -> c3 -> e5 -> g7 +3**
**a1 -> c3 -> e5 -> g7 -> e9 +4**
**a1 -> c3 -> e5 -> g7 -> e9 -> c7 +5**
**a1 -> c3 -> e5 -> g7 -> e9 -> c7 -> e5 +6**
**a1 -> c3 -> e5 -> g7 -> e9 -> c7 -> f4 +6**
**a1 -> c3 -> e5 -> g7 -> e9 -> b6 +5**
**a1 -> c3 -> e5 -> g7 -> e9 -> a5 +5**
**a1 -> c3 -> e5 -> g7 -> d10 +4**
**g1 -> f2**
**g1 -> e3**
**g1 -> c5 +1**
**g1 -> c5 -> e7 +2**
**g1 -> c5 -> e7 -> c9 +3**
**g1 -> c5 -> e7 -> g5 +3**
**g1 -> c5 -> e7 -> g9 +3**
**g1 -> c5 -> e7 -> g9 -> i7 +4**
**g1 -> c5 -> e7 -> g9 -> j6 +4**
**g1 -> c5 -> e7 -> h10 +3**
**g1 -> c5 -> e7 -> h10 -> j8 +4**
**g1 -> b6 +1**
**g1 -> b6 -> e9 +2**
**g1 -> b6 -> e9 -> g7 +3**
**g1 -> b6 -> e9 -> g7 -> e5 +4**
**g1 -> b6 -> e9 -> g7 -> e5 -> c7 +5**
**g1 -> b6 -> e9 -> g7 -> e5 -> b8 +5**
**g1 -> b6 -> e9 -> g7 -> e5 -> a9 +5**
**g1 -> b6 -> e9 -> g7 -> d4 +4**
**g1 -> b6 -> e9 -> g7 -> c3 +4**
**g1 -> b6 -> e9 -> h6 +3**
**g1 -> b6 -> e9 -> i5 +3**
**g1 -> b6 -> e9 -> j4 +3**
**g1 -> b6 -> f10 +2**
**g1 -> b6 -> f10 -> i7 +3**
**g1 -> b6 -> f10 -> j6 +3**
**g1 -> a7 +1**
**g3 -> i5 +1**
**g3 -> i5 -> e9 +2**
**g3 -> i5 -> e9 -> c7 +3**
**g3 -> i5 -> e9 -> c7 -> e5 +4**
**g3 -> i5 -> e9 -> c7 -> e5 -> c3 +5**
**g3 -> i5 -> e9 -> c7 -> e5 -> g7 +5**
**g3 -> i5 -> e9 -> c7 -> f4 +4**
**g3 -> i5 -> e9 -> c7 -> g3 +4**
**g3 -> i5 -> e9 -> b6 +3**
**g3 -> i5 -> e9 -> b6 -> e3 +4**
**g3 -> i5 -> e9 -> b6 -> f2 +4**
**g3 -> i5 -> e9 -> a5 +3**
**g3 -> i5 -> d10 +2**
**g3 -> j6 +1**
**g3 -> j6 -> g9 +2**
**g3 -> j6 -> g9 -> e7 +3**
**g3 -> j6 -> g9 -> e7 -> g5 +4**
**g3 -> j6 -> g9 -> e7 -> h4 +4**
**g3 -> j6 -> g9 -> e7 -> c9 +4**
**g3 -> j6 -> g9 -> e7 -> c5 +4**
**g3 -> j6 -> g9 -> e7 -> c5 -> e3 +5**
**g3 -> j6 -> g9 -> e7 -> c5 -> f2 +5**
**g3 -> j6 -> g9 -> e7 -> b4 +4**
**g3 -> j6 -> g9 -> e7 -> a3 +4**
**g3 -> j6 -> g9 -> e7 -> a3 -> c1 +5**
**g3 -> j6 -> f10 +2**
**g3 -> j6 -> f10 -> c7 +3**
**g3 -> j6 -> f10 -> c7 -> e5 +4**
**g3 -> j6 -> f10 -> c7 -> e5 -> c3 +5**
**g3 -> j6 -> f10 -> c7 -> e5 -> g7 +5**
**g3 -> j6 -> f10 -> c7 -> e5 -> g7 -> e9 +6**
**g3 -> j6 -> f10 -> c7 -> e5 -> g7 -> d10 +6**
**g3 -> j6 -> f10 -> c7 -> e5 -> g7 -> j10 +6**
**g3 -> j6 -> f10 -> c7 -> e5 -> h8 +5**
**g3 -> j6 -> f10 -> c7 -> e5 -> h8 -> j10 +6**
**g3 -> j6 -> f10 -> c7 -> f4 +4**
**g3 -> j6 -> f10 -> c7 -> g3 +4**
**g3 -> j6 -> f10 -> b6 +3**
**g3 -> j6 -> f10 -> b6 -> e3 +4**
**g3 -> j6 -> f10 -> b6 -> f2 +4**
**g3 -> j6 -> f10 -> a5 +3**
**g3 -> f4**
**g3 -> e5**
**g3 -> c7 +1**
**g3 -> c7 -> e9 +2**
**g3 -> c7 -> e9 -> g7 +3**
**g3 -> c7 -> e9 -> g7 -> e5 +4**
**g3 -> c7 -> e9 -> g7 -> e5 -> c3 +5**
**g3 -> c7 -> e9 -> h6 +3**
**g3 -> c7 -> e9 -> i5 +3**
**g3 -> c7 -> e9 -> i5 -> g3 +4**
**g3 -> c7 -> e9 -> i5 -> f2 +4**
**g3 -> c7 -> e9 -> i5 -> f2 -> c5 +5**
**g3 -> c7 -> e9 -> i5 -> f2 -> b6 +5**
**g3 -> c7 -> e9 -> i5 -> f2 -> a7 +5**
**g3 -> c7 -> e9 -> i5 -> e1 +4**
**g3 -> c7 -> e9 -> j4 +3**
**g3 -> c7 -> f10 +2**
**g3 -> c7 -> f10 -> i7 +3**
**g3 -> c7 -> f10 -> j6 +3**
**g3 -> c7 -> f10 -> j6 -> g3 +4**
**g3 -> c7 -> f10 -> j6 -> f2 +4**
**g3 -> c7 -> f10 -> j6 -> f2 -> c5 +5**
**g3 -> c7 -> f10 -> j6 -> f2 -> c5 -> g9 +6**
**g3 -> c7 -> f10 -> j6 -> f2 -> c5 -> h10 +6**
**g3 -> c7 -> f10 -> j6 -> f2 -> c5 -> h10 -> j8 +7**
**g3 -> c7 -> f10 -> j6 -> f2 -> b6 +5**
**g3 -> c7 -> f10 -> j6 -> f2 -> a7 +5**
**g3 -> c7 -> f10 -> j6 -> e1 +4**
**g3 -> b8 +1**
**g3 -> a9 +1**
**g3 -> f2**
**g3 -> e1**
**i3 -> j4**
**i3 -> g5 +1**
**i3 -> g5 -> e7 +2**
**i3 -> g5 -> e7 -> g9 +3**
**i3 -> g5 -> e7 -> g9 -> i7 +4**
**i3 -> g5 -> e7 -> g9 -> j6 +4**
**i3 -> g5 -> e7 -> h10 +3**
**i3 -> g5 -> e7 -> h10 -> j8 +4**
**i3 -> g5 -> e7 -> c5 +3**
**i3 -> g5 -> e7 -> c5 -> e3 +4**
**i3 -> g5 -> e7 -> c5 -> f2 +4**
**i3 -> g5 -> e7 -> b4 +3**
**i3 -> g5 -> e7 -> a3 +3**
**i3 -> g5 -> e7 -> a3 -> c1 +4**
**i3 -> g5 -> e7 -> c9 +3**
**Celkem ruznych tahu: 141**

----------

**Velikost hraci plochy:**
5
**Pozice kamenu:**
W c3
b b2
b d4
**c3 -> e5 +1**
**c3 -> b4**
**c3 -> a5**
**c3 -> a1 +1**
**c3 -> d2**
**c3 -> e1**
**Celkem ruznych tahu: 6**

----------

**Velikost hraci plochy:**
10
**Pozice kamenu:**
w a2
**Nespravny vstup.**

----------

**Velikost hraci plochy:**
27
**Nespravny vstup.**