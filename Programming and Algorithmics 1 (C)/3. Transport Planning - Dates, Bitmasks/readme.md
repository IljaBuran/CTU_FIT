
Úkolem je realizovat funkce (ne celý program, pouze funkce), které budou usnadňovat plánování dopravy.

Předpokládáme dopravní spojení dvou míst A a B. V jeden den jede z A do B nula nebo více spojů. Počet spojů je daný celým číslem. Dále můžeme omezit konkrétní dny v týdnu, ve kterých spoj jezdí. Například můžeme definovat, že spojení je v provozu v pondělí, v pátek a v sobotu (v ostatní dny týdne ne). Pro zjednodušení uvažujeme, že počet spojů je daný jedním celým číslem  perWorkDay:

-   pokud je spojení v provozu v daný všední den, pak jede  perWorkDay  spojů,
-   pokud je spojení v provozu v sobotu, pak jede  perWorkDay / 2  spojů, desetinnou část zaokrouhlíme nahoru,
-   pokud je spojení v provozu v neděli, pak jezdí  perWorkDay / 3  spojů, desetinnou část zaokrouhlíme nahoru.

Pro plánování potřebujeme vědět, kolik spojů bude celkem potřeba objednat v zadaném časovém intervalu. Dále se hodí i opačná funkce na výpočet časového intervalu, který lze pokrýt objednaným počtem spojů. Tyto výpočty budou realizované požadovanými funkcemi:

countConnections ( from, to, perWorkDay, dowMask )

funkce dostane v parametrech časový interval  <from; to>, počet spojů během jednoho všedního dne  perWorkDay  a masku dní v týdnu, kdy je spoj v provozu  dowMask. Na základě těchto parametrů funkce vypočte počet spojů, které je potřeba objednat pro pokrytí zadaného intervalu dní. Interval chápeme jako uzavřený, tedy obsahuje celý první i celý poslední den. Pokud jsou parametry neplatné, funkce vrátí hodnotu  -1. Za chybu považujeme:

-   neplatné datum  from  nebo  to  nebo
-   počátek intervalu  from  nastane po konci intervalu  to  (tj. nesprávné je  from > to).

Implementace této funkce je Vaším úkolem.

endDate ( from, connections, perWorkDay, dowMask )

funkce dostane počáteční datum  from, počet objednaných spojení  connections, počet spojů během jednoho všedního dne  perWorkDay  a masku dní v týdnu, kdy je spoj v provozu  dowMask. Na základě těchto parametrů funkce vypočte datum posledního dne, který jsme se zadaným počtem spojeni schopni pokrýt (tj. pro další den již nebude dostatek objednaných spojení). Návratovou hodnotou je nalezené datum nebo datum  0000-00-00  pro neplatnou kombinaci parametrů:

-   neplatné datum  from,
-   záporný počet objednaných spojení  connections,
-   počet objednaných spojení  connections  nestačí ani na den  from,
-   nulová hodnota spojení  perWorkDay  nebo
-   prázdná maska  dowMask.

Implementace této funkce je Vaším úkolem.

TDATE

je struktura reprezentující datum. Tvoří ji složky rok, měsíc a den. Struktura je deklarovaná v testovacím prostředí, Vaše implementace ji může/musí používat. Nelze ale měnit deklaraci struktury.

makeDate(y, m, d)

pomocná funkce deklarovaná v testovacím prostředí. Funkci můžete použít pro usnadnění ladění. Implementaci nelze změnit.

konstanty  DOW_MON,  DOW_TUE, ...,  DOW_ALL

konstanty jsou deklarované v testovacím prostředí a nelze je měnit. Používají se pro masky dní, ve kterých spoj jezdí.