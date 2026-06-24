
# Zadání

Vaším úkolem je realizovat dvě (či více) funkcí, (ne celý program), které dokáží zašifrovat a dešifrovat obrázkový soubor ve formátu  [TGA](http://www.paulbourke.net/dataformats/tga/).

Pro naši úlohu budeme uvažovat zjednodušenou formu obrázku:

-   Povinná hlavička: 18 bytů - tyto bajty nijak nemodifikujte, jen je překopírujte do zašifrovaného obrázku.
-   Volitelná část hlavičky: velikost se vypočítá z povinné části hlavičky - tuto část hlavičky považujte za obrázková data, tj. beze změn je  **zašifrujete společně**  s obrázkovými daty.
-   Obrázková data: zbytek.

## Rozhraní funkcí

int encrypt_data (const string & in_filename, const string & out_filename, crypto_config & config);
int decrypt_data (const string & in_filename, const string & out_filename, crypto_config & config);

-   `in_filename`  obsahuje vstupní jméno souboru,
-   `out_filename`  obsahuje výstupní jméno souboru,
-   `config`  je datová struktura  `crypto_config`  popsaná níže.
-   Návratová hodnota je  `EXIT_SUCCESS`  v případě úspěchu,  `EXIT_FAILURE`  v opačném případě. K neúspěchu dochází, pokud je soubor nějakým způsobem nevalidní (schází povinná hlavička, nepodaří se otevřít, číst, zapsat, …) nebo se nepodaří opravit nevalidní konfiguraci  `crypto_config`.

Funkce `encrypt_data` zašifruje soubor (vyjma 18 bajtové hlavičky) a funkce `decrypt_data` dešifruje zašifrovaná data (rovněž vyjma hlavičky, která není šifrovaná). Výstupní soubor dešifrování by měl mít bitovou shodu se vstupním souborem pro šifrování.

Datová struktura  `crypto_config`  obsahuje:

-   zvolenou symetrickou šifru zadanou pomocí jejího názvu,
-   sdílený klíč a jeho velikost,
-   inicializační vektor (IV) a jeho velikost.

Vámi implementované funkce musí ověřit, zda je `config` zadán validně (tedy zda je klíč a IV dostatečně dlouhý pro zadanou šifru):

-   V případě, že  `encrypt_data`  dostane na vstup nevalidní  `config`, pak daný  `config`  opraví (tzn. vygeneruje klíč, resp. IV, a nastaví délku klíče, resp. IV, v  `configu`).
    -   Dejte si pozor, že ne každá symetrická šifra využívá IV. V takovém případě nesmíte vygenerovat nový.
    -   Pokud budete generovat klíč nebo IV, musíte ho vygenerovat kryptograficky bezpečným generátorem náhodných čísel.
    -   Pokud budete generovat klíč nebo IV, musíte ho uložit do  `config`  a stejně tak i jeho délku.
    -   Pokud klíč nebo IV je delší než potřebuje daná šifra, negenerujte nový. Nový generujete, jen a pouze, když jsou délky nedostatečné.
-   V případě, že  `decrypt_data`  dostane na vstup nevalidní (nedostačujicí)  `config`, pak nemůže dešifrovat data a vrátí EXIT_FAILURE.