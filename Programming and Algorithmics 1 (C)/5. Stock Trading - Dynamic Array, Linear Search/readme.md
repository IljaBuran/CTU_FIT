
Úkolem je realizovat program, který bude počítat zisk a ztrátu při obchodování s akciemi.

Předpokládáme, že zpracováváme cenu akcie v čase. Cenu akcie se postupně dozvídáme pro jednotlivé časové okamžiky. Pro známé hodnoty ceny v minulosti chceme vypočítat maximální možný zisk a maximální možnou ztrátu. Při výpočtu dostaneme zadaný časový interval, ve kterém obchod proběhne. Akcii můžeme jednou nakoupit (někdy v zadaném časovém intervalu, za cenu platnou v okamžiku nákupu) a jednou prodat (opět v zadaném časovém intervalu, za cenu platnou v dobu prodeje). Při obchodování nelze shortovat, tj. musíme nejprve akcii nakoupit a teprve pak prodat.

Vstupem programu je zadání ceny akcie prokládané dotazy na možný zisk a ztrátu. Cena akcie je zadána ve tvaru  + cena, např.  + 200. Dotaz na zisk/ztrátu má podobu  ? from to, např.  ? 5 10. Ceny akcie a dotazy jsou zadávané postupně, zadávání končí dosažením konce vstupu (EOF). Následuje ukázkový běh #1 s dodaným vysvětlením (za znakem #):

+ 20                 # cena v čase 0
+ 30                 # cena v čase 1
+ 40                 # cena v čase 2
+ 10                 # cena v čase 3
? 0 2                # hledání max. zisku/ztráty pro období <0;2>
? 0 3                # hledání max. zisku/ztráty pro období <0;3>
+ 50                 # cena v čase 4
? 0 3                # hledání max. zisku/ztráty pro období <0;3>
? 1 4                # hledání max. zisku/ztráty pro období <1;4>

Výstupem programu je řešení jednotlivých zadaných problémů. Pro každý zadaný problém je zobrazena informace o maximálním možném zisku (spolu s časovým okamžikem nákupu a prodeje akcie) a informace o maximální možné ztrátě (opět s časovými okamžiky nákupu a prodeje). Pokud v zadaném intervalu nelze realizovat zisk/ztrátu, je výsledkem hodnota  N/A, viz ukázka.

Pokud vstup není platný, program tuto situaci detekuje, vypíše chybové hlášení a ukončí se. Formát chybového hlášení je opět uveden v ukázkách níže. Za chybu je považováno:

-   nerozpoznaný vstup (nezačíná znakem + ani ?),
-   neplatné zadání ceny akcie (nečíselné, záporná cena),
-   neplatné zadání dotazu (neobsahuje dvě celá čísla, počátek intervalu je záporný, konec intervalu je větší nebo roven počtu dosud známých cen akcií, počátek intervalu je větší než konec intervalu).

**Ukázka práce programu:**  

----------

**Ceny, hledani:**
+ 20
+ 30
+ 40
+ 10
? 0 2
**Nejvyssi zisk: 20 (0 - 2)**
**Nejvyssi ztrata: N/A**
? 0 3
**Nejvyssi zisk: 20 (0 - 2)**
**Nejvyssi ztrata: 30 (2 - 3)**
+ 50
? 0 3
**Nejvyssi zisk: 20 (0 - 2)**
**Nejvyssi ztrata: 30 (2 - 3)**
? 1 4
**Nejvyssi zisk: 40 (3 - 4)**
**Nejvyssi ztrata: 30 (2 - 3)**

----------

**Ceny, hledani:**
+ 100
+ 110
+ 105
+ 120
+ 70
+ 105
+ 50
+ 85
+ 40
? 0 8
**Nejvyssi zisk: 35 (4 - 5)**
**Nejvyssi ztrata: 80 (3 - 8)**
? 0 6
**Nejvyssi zisk: 35 (4 - 5)**
**Nejvyssi ztrata: 70 (3 - 6)**
? 3 5
**Nejvyssi zisk: 35 (4 - 5)**
**Nejvyssi ztrata: 50 (3 - 4)**
? 2 8
**Nejvyssi zisk: 35 (4 - 5)**
**Nejvyssi ztrata: 80 (3 - 8)**
? 7 7
**Nejvyssi zisk: N/A**
**Nejvyssi ztrata: N/A**

----------

**Ceny, hledani:**
+ 10
+ 20
+ 30
+ 10
+ 20
+ 30
+ 5
+ 25
? 0 7
**Nejvyssi zisk: 20 (0 - 2)**
**Nejvyssi ztrata: 25 (2 - 6)**
? 2 7
**Nejvyssi zisk: 20 (3 - 5)**
**Nejvyssi ztrata: 25 (2 - 6)**
? 4 7
**Nejvyssi zisk: 20 (6 - 7)**
**Nejvyssi ztrata: 25 (5 - 6)**
+ 50
? 0 8
**Nejvyssi zisk: 45 (6 - 8)**
**Nejvyssi ztrata: 25 (2 - 6)**

----------

**Ceny, hledani:**
+ 10
* 20
**Nespravny vstup.**

----------

**Ceny, hledani:**
+ 10
+ 20
+ 30
? 0 2
**Nejvyssi zisk: 20 (0 - 2)**
**Nejvyssi ztrata: N/A**
? 0 3
**Nespravny vstup.**