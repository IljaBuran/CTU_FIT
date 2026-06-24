# Daňový registr

Úkolem je realizovat třídu **CTaxRegister**, která bude implementovat

Pro plánované důslednější potírání daňových úniků je potřeba vybudovat databázi všech obyvatel, ve které budou archivované jejích příjmy a výdaje. Předpokládáme, že v databázi je zaveden každý občan v okamžiku jeho narození. Od té doby se mu počítají všechny příjmy a všechny výdaje. Záznam je z databáze odstraněn v okamžiku jeho úmrtí. Občan je identifikován svým jménem, adresou a číslem účtu. Číslo účtu je unikátní přes celou databázi. Jména a adresy se mohou opakovat, ale dvojice (jméno, adresa) je opět v databázi unikátní. Tedy v databázi může být mnoho jmen *Jan Novak*, mnoho lidí může mít adresu *Hlavni 60*, ale *Jan Novak* bydlící na adrese *Hlavni 60* může být v databázi pouze jeden.

### **CTaxRegister**
Veřejné rozhraní třídy **CTaxRegister** obsahuje následující:

- Konstruktor bez parametrů. Tento konstruktor inicializuje instanci třídy tak, že vzniklá instance je zatím prázdná (neobsahuje žádné záznamy).
- Destruktor. Uvolňuje prostředky, které instance alokovala.
- Metoda birth(name, addr, acct) přidá do existující databáze další záznam. Parametry name a addr reprezentují jméno a adresu občana, parametr acct udává číslo účtu. Metoda vrací hodnotu true, pokud byl záznam přidán, nebo hodnotu false, pokud přidán nebyl (protože již v databázi existoval záznam se stejným jménem a adresou, nebo záznam se stejným číslem účtu).
- Metoda death (name, addr) odstraní záznam z databáze. Parametrem je jednoznačná identifikace pomocí jména a adresy. Pokud byl záznam skutečně odstraněn, vrátí metoda hodnotu true. Pokud záznam neodstraní (protože neexistoval občan s touto identifikací), vrátí metoda hodnotu false.
- Metody income zaznamenají na účet pro daného občana příjem ve výši amount. Varianty jsou dvě - občan je buď identifikován svým jménem a adresou, nebo identifikátorem jeho účtu. Pokud metoda uspěje, vrací true, pro neúspěch vrací false (neexistující občan/účet).
- Metody expense zaznamenají na účet daného občana výdaj ve výši amount. Varianty jsou dvě - občan je buď identifikován svým jménem a adresou, nebo identifikátorem jeho účtu. Pokud metoda uspěje, vrací true, pro neúspěch vrací false (neexistující občan/účet).
- Metoda audit ( name, addr, acct, sumIncome, sumExpense ) vyhledá pro občana se zadaným jménem a adresou a všechny dostupné informace (účet, součet příjmů a součet výdajů). Nalezené informace uloží do zadaných výstupních parametrů. Metoda vrátí true pro úspěch, false pro selhání (neexistující dvojice jméno + adresa).
- Metoda listByName vrátí instanci třídy **CIterator**. Vrácený objekt má rozhraní podle seznamu níže. Jeho pomocí půjde procházet záznamy v registru od počátku do konce. Při načítání budou záznamy procházené v pořadí zvětšujícího se jména (seřazená podle jména), pro stejná jména budou nejdříve vracené záznamy seřazené podle adresy.


### **CIterator**
Veřejné rozhraní třídy **CIterator** obsahuje následující:

- Metoda atEnd vrací hodnotu true pokud bylo dosaženo konce registru (tedy iterátor již neodkazuje na platný záznam) nebo false pokud iterátor ještě nedosáhl konce registru (odkazuje na platný záznam).
- Metoda next posune iterátor na další záznam.
- Metoda name zjistí jméno osoby na aktuální pozici iterátoru.
- Metoda addr zjistí adresu osoby na aktuální pozici iterátoru.
- Metoda account zjistí identifikátor účtu osoby na aktuální pozici iterátoru.