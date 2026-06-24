
Regulární výrazy jsou často používané pro kontrolu, zda nějaké slovo splňuje určitá kritéria. Například, chceme-li validovat emailovou adresu či IP adresu, můžeme napsat regulární výraz, který popisuje všechny emailové či IP adresy a nechat naší oblíbenou knihovnu (či nástroj) pro práci s regulárními výrazy zvalidovat různé vstupní řetězce proti výrazu. Cílem této úlohy bude napsat si takovou funkcionalitu.

----------

Jednotlivé regulární výrazy jsou reprezentovány vlastními třídami ve jmenném prostoru (namespace)  `regexp`:

-   `Empty`  reprezentuje prázdný regulární výraz (∅),
-   `Epsilon`  reprezentuje prázdné slovo (ε),
-   `Symbol`  reprezentuje jeden symbol regulárního výrazu,
-   `Alternation`  reprezentuje regulární výraz sjednocení pomocí binárního operátoru  `+`, na levé i pravé straně se vyskytují regulární výrazy,
-   `Concatenation`  reprezentuje regulární výraz zřetězení pomocí binárního operátoru  `.`, na levé i pravé straně se vyskytují regulární výrazy,
-   `Iteration`  reprezentuje unární operátor  `*`, jeho potomkem je regulární výraz.

Regulární výrazy jsou pak reprezentovány jako stromy složené z těchto tříd. Třída  `regexp::RegExp`  pak reprezentuje libovolný výraz. Všechny tyto třídy jsou definovány v testovacím prostředí a není potřeba ani dovoleno je implementovat.

----------

Vlastní "knihovna" pro matchování regulárních výrazů bude reprezentována funkcí  `std::set<size_t> wordsMatch(const regexp::RegExp&, const std::vector<Word>&)`. Tato funkce očekává jako parametry regulární výraz a vektor slov, která se mají proti výrazu otestovat. Návratovou hodnotou je množina indexů slov (indexujeme samozřejmě od nuly), která patří do jazyka daného regulárním výrazem. Nedodržení výše předepsaného rozhraní povede k chybě při kompilaci.

Slovo (`Word`) je reprezentováno jako vektor znaků typu  `uint8_t`.