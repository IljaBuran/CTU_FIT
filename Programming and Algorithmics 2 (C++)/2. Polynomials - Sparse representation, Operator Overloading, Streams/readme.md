# Polynomy

Úkolem je realizovat třídu **CPolynomial**, která bude reprezentovat polynomy.

Třída **CPolynomial** ukládá polynom pomocí koeficientů u jednotlivých jeho mocnin. Předpokládáme koeficienty v podobě desetinných čísel typu double. Pomocí rozhraní (většina rozhraní má podobu přetížených operátorů) dokáže tato třída s polynomy pracovat. Realizovaná třída musí splňovat následující rozhraní:

- *konstruktor implicitní*:  
inicializuje objekt, který bude reprezentovat polynom o hodnotě 0.
- *kopírující konstruktor*:  
bude implementován, pokud to vnitřní struktury Vaší třídy vyžadují.
- *destruktor*  
bude implementován, pokud to vnitřní struktury Vaší třídy vyžadují.
- *přetížený operátor=*  
bude umožňovat kopírování polynomů (pokud automaticky generovaný operátor = nevyhovuje).
- *operátor<<*  
bude umožňovat výstup objektu do C++ streamu. Výpis bude realizován v kompaktní podobě:  
    - členy jsou zobrazované směrem od nejvyšší mocniny,
    - členy s nulovým koeficientem nebudou zobrazované,
    - členy s koeficientem +1 či -1 nebudou zobrazovat číslo 1, zobrazí pouze příslušnou mocninu x,
    - ve výpisu nejsou zbytečná znaménka - (tedy např. x - 9 nebo - x^2 + 4 je správné, ale x + (-9) je špatně),
    - nulový polynom se zobrazí jako samostatné číslo 0.
- *operátor\**  
umožní vynásobit polynom desetinným číslem nebo dva polynomy,
operátor *=  
umožní vynásobit polynom desetinným číslem nebo dva polynomy,
- *operátory== a !=*  
umožní porovnat polynomy na přesnou shodu (neuvažujeme epsilon toleranci),
- *operátor[]*  
umožní nastavit/zjistit hodnotu koeficientu u zadané mocniny polynomu. Čtecí varianta musí fungovat i pro konstantní polynomy,
- *operátor()*  
umožní vyhodnotit hodnotu polynomu pro zadanou hodnotu x (x je desetinné číslo),
- *degree()*  
metoda zjistí stupeň polynomu (např. x^3+4 má stupeň 3, 5 má stupeň 0, 0 má stupeň 0).
- *přetypování na typ bool*  
vrací true, pokud se nejedná o nulový polynom (má nějaký koeficient nenulový).
- *operátor!*  
vrací true, pokud se jedná o nulový polynom (všechny koeficienty jsou nulové).
- *Nepovinný manipulátor poly_var:* /* Nedokončené */  
manipulátorem půjde změnit jméno proměnné ve výpisu polynomu. Výchozí jméno proměnné je x, použitím manipulátoru půjde nastavit na libovolný řetězec,
