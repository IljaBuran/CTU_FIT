# Registr obyvatel
Úkolem je realizovat třídu **CRegister**, která bude implementovat registr obyvatelstva.

Uvažovaný registr je specifický tím, že chceme ukládat všechny změny (všechny nahlášené změny bydliště daného člověka). Dále, pro účely archivace chceme ukládat kopie registru.

Úloha má procvičit práci s kopírováním objektů. Z tohoto důvodu jsou v úloze potlačené části standardní C++ knihovny, zejména STL a datový typ C++ string.

Požadovaná třída **CRegister** má následující rozhraní:

- *implicitní konstruktor*:  
vytvoří prázdnou instanci registru,
- *kopírující konstruktor*:  
vytvoří hlubokou kopii instance. Implementujte jej, pokud kompilátorem automaticky vytvořený kopírující konstruktor nevyhovuje (asi bude potřeba),
- *destruktor*:  
uvolní prostředky alokované instancí.
- *operátor =*:  
zkopíruje obsah jedné instance do druhé (hluboká kopie). Implementujte jej, pokud kompilátorem automaticky vytvořený operátor = nevyhoví,
- *add(id,name,surname,date,street,city)*:  
metoda přidá zadaného člověka do databáze a nastaví u něj adresu pobytu. Parametrem je 6 řetězců s následujícím významem:
id udává jednoznačný identifikátor člověka. Jedná se o řetězec, můžete předpokládat, že je vždy ve formátu "XXXXXX/YYYY",
name udává jméno člověka. Jméno může být libovolné, libovolně dlouhé.
surname udává příjmení člověka. Může být libovolné, libovolně dlouhé.
date udává datum počátku platnosti místa pobytu. Jedná se o řetězec, můžete předpokládat, že je vždy ve formátu "YYYY-MM-DD".
street udává adresu pobytu, řetězec, libovolný obsah a délka.
city udává adresu místa pobytu, řetězec, libovolný obsah a délka.
Metoda add vrací hodnotu true pro signalizaci úspěchu, false pro neúspěch (osoba se stejným id již byla zadaná).
- *resettle(id,date,street,city)*:  
metoda přidá místo pobytu do záznamu daného člověka. Význam parametrů je stejný jako u metody add. Metoda vrací true pokud operace uspěje, false pokud selže (člověk se zadaným id neexistuje nebo daný člověk v ten samý den již dříve oznámil jinou adresu (tedy v jeden den se lze přestěhovat pouze jedenkrát). Pozor - informace o změně adresy nemusí být zpracovávány v pořadí vzrůstajícího času.
- *print(stream,id)*:  
metoda zobrazí záznam pro člověka zadaného id do zadaného výstupního proudu stream. Metoda vrací true pro signalizaci úspěchu, false pro neúspěch (člověk se zadaným id neexistuje). Formát výstupu je uveden v ukázkách běhu, adresy trvalého bydliště jsou vypisované v rostoucím pořadí dle data stěhování.