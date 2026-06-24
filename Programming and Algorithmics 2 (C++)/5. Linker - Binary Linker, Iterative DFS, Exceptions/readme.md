# Linker

Úkolem je realizovat třídu, která implementuje zjednodušenou variantu linkeru.

Linker je součást kompilátoru, která má na vstupu object soubory, ze kterých vyrobí finální spustitelný program. Každý object soubor obsahuje část přeloženého zdrojového kódu, zpravidla jednoho modulu. Object soubory obsahují tabulky importovaných a exportovaných symbolů. Linker tyto object soubory načte, propojí požadované importy s dostupnými exporty, zkontroluje jejich správnost, dosadí vypočtené adresy a výsledný soubor uloží na disk. Ve zjednodušené podobě to bude práce pro implementovanou třídu.

Základem object souboru je přeložený strojový kód. Ten obsahuje instrukce procesoru (v úloze uvažujeme pouze funkce). Bajty, které tvoří instrukce, jsou uložené v jednom soubislém bloku. V instrukcích ale mohou chybět některé adresy - bajty tvořící adresy zatím obsahují neplatné hodnoty, často nuly. Linker tyto bajty přepíše platnými adresami, které vzniknou po serializaci funkcí do výsledného souboru. Přeložený kód v naší úloze představuje posloupnost náhodných bajtů nějaké délky, nejedná se o platné instrukce nějakého procesoru.

Linker potřebuje znát jména funkcí, které se v daném object souboru nachází. K tomu slouží tabulka exportů. V úloze má tabulka zjednodušenou strukturu - obsahuje pouze jméno funkce (symbolu) a její pozici v přeloženém kódu (offset, počítaný v bajtech od počátku bloku přeloženého kódu). Object soubor může obsahovat více funkcí, funkce se v přeloženém kódu nepřekrývají. Proto lze snadno identifikovat začátek a konec funkce v přeloženém kódu; funkce končí tam, kde začíná funkce následující. Pozor, pořadí funkcí v tabulce exportů nemusí odpovídat pořadí funkcí v přeloženém kódu (viz obrázek).

Linker dále potřebuje znát pozice v přeloženém kódu, kde bude nahrazovat odkazy na používané funkce. K tomu slouží tabulka importů. Pro každý importovaný symbol je uvedeno jeho jméno, počet výskytů a jednotlivé pozice v přeloženém kódu, kde má být adresa zapsána (opět ofset v bajtech od počátku přeloženého kódu).

Linkování dostane na vstupu jméno počáteční funkce. Je to obdoba funkce main, ale jméno může být libovolný řetězec. Tato funkce bude zkopírovaná do výsledného souboru jako první, na jeho začátek. Zkopíruje se tělo této funkce, tedy odpovídající bajty přeloženého kódu. Tato funkce může potřebovat volat další funkce. To, které funkce to jsou, lze snadno poznat. Do těla "main" funkce budou odkazovat odkazy z tabulky importů, tedy tyto funkce jsou volané z "main". Do výsledného souboru tedy bude potřeba nakopírovat tyto odkazované funkce (v nějakém pořadí za první "main") a ve zkopírované main odpovídajícím způsobem vyplnit odkazy na tyto funkce. Celý postup je potřeba aplikovat opakovaně, protože každá zkopírovaná funkce může mít své další závislosti. Linkování se snaží, aby výsledný soubor byl co nejkratší. Tedy stejnou funkci nekopíruje opakovaně a mezi funkce nevkládá žádné bajty navíc. Dále, do výsledného souboru nejsou ukládané nedostupné funkce. Pořadí funkcí ve výsledném souboru není určeno (zvolte si), ale první musí být funkce odpovídající zadanému vstupnímu bodu ("main").

Pro zjednodušení předpokládáme, že všechny adresy mají velikost 32 bitů, jsou kódované v little-endian pořadí bajtů (intel) a všechny adresy ve výsledném souboru jsou absolutní (pozice od začátku výstupního souboru).

**Formát vstupního object souboru:**
| Offset | Velikost | Význam                                                              |
| ------ | -------- | ------------------------------------------------------------------- |
| 0000   | 4B       | počet exportovaných funkcí                                         |
| 0004   | 4B       | počet importovaných funkcí                                         |
| 0008   | 4B       | celkový počet bajtů strojového kódu                                |
| 000C   | ?        | tabulka exportovaných funkcí (počet dán číslem na offsetu 0000)    |
| ????   | ?        | tabulka importovaných funkcí (počet dán číslem na offsetu 0004)    |
| ????   | ?        | blok přeloženého kódu funkcí (počet bajtů dán číslem na offsetu 0008) |

**Exportovaná funkce:**
| Offset od začátku bloku | Velikost | Význam                                                                        |
| ----------------------- | -------- | ----------------------------------------------------------------------------- |
| +0000                   | 1B       | počet znaků ve jménu funkce                                                   |
| +0001                   | ?        | znaky jména funkce                                                            |
| +????                   | 4B       | umístění vstupního bodu funkce (v bajtech od začátku bloku přeloženého kódu) |

**Importovaná funkce:**
| Offset od začátku bloku | Velikost | Význam                                                                                         |
| ----------------------- | -------- | ---------------------------------------------------------------------------------------------- |
| +0000                   | 1B       | počet znaků ve jménu funkce                                                                     |
| +0001                   | ?        | znaky jména funkce                                                                              |
| +????                   | 4B       | N – počet odkazů na tuto funkci (kolikrát je volána)                                           |
| +????                   | N×4B     | seznam offsetů (každý 4 B) s umístěním volání funkce (v bajtech od začátku bloku přeloženého kódu) |

![image](images/pic.png)

Požadovaná třída ***CLinker*** má rozhraní:

**implicitní konstruktor, destruktor**
- pracují v obvyklém významu  

**kopírovací konstruktor a operátor=**
- nejsou používané, mohou být potlačené (=delete).

**addFile(objFileName)**
- přidá další vstupní object soubor, jméno souboru je udané parametrem. Metoda soubor může načíst, případně si jen jméno souboru poznamená pro další použití. Metoda může vyhodit výjimku std::runtime_error, pokud v ní načítáte object soubory a čtení selže.

**linkOutput ( outFileName, entryPoint )**
- metoda fakticky vytvoří výsledný slinkovaný soubor outFileName. Druhý parametr udává jméno počáteční funkce (tj. ekvivalent funkce "main"). Metoda nic nevrací, případnou chybu oznamuje vyhozením výjimky std::runtime_error.

