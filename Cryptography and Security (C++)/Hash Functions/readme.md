
# Zadání

Vaším úkolem je realizovat funkci (či sadu funkcí, ne celý program), která nalezne  **libovolnou**  zprávu, jejíž hash (SHA-256 nebo libovolná hashovací funkce) začíná zleva na posloupnost zadaných bitů - prefixu.

## Rozhraní funkce

Deklarace vámi implementované funkce je:

`int findHash (const uint8_t prefix[], const unsigned int prefixBitLen, std::unique_ptr<uint8_t[]>& outputMessage, size_t& outputMessageLen, std::unique_ptr<uint8_t[]>& outputHash, size_t& outputHashLen, const char hashName[] = "sha256");`

## Vstupní parametry

-   `prefix`  obsahuje binární pole prefixu
-   `prefixBitLen`  obsahuje délku hledané kolize v  **bitech**
-   `hashName`  je název hashovací funkce (kompatibilní s  `EVP_get_digestbyname`)

## Výstupní parametry

-   `outputMessage`  pole pro navrácení nalezené zprávy
-   `outputMessageLen`  délka vrácené zprávy v  **bytech**
-   `outputHash`  pole pro navrácení nalezené hashe
-   `outputHashLen`  délka vrácené hashe v  **bytech**

Všechna pole (`prefix`, `outputMessage` a `outputHash`) jsou binární pole. Nejedná se o pole hexadecimálních znaků ani o C-string. Zároveň jsou obaleny `std::unique_ptr`, který ulehčuje dynamické odalokování na straně progtestu. Váš kód alokuje pole potřebné délky (např. pomocí `std::make_unique<uint8_t[]>(sizeByte);`), které vrátíte testovacímu kódu, kde se pole odalokuje automaticky po kontrole výstupu.

Délky polí (`outputMessageLen`  a  `outputHashLen`) jsou v bytech. Není tedy možnost vrátit zprávu nebo hash, které nejsou zarovnány na byte. Např. pokud je  `outputMessageLen=1`, pak vaše zpráva je dlouhá 8 bitů.

## Upřesnění kontroly prefixu

Prefix kontrolujte od nejnižších bytových pozic (v pořadí: prefix[0], prefix[1], ...) a pomocí most significant bit (bity na vyšších pozicích, v pořadí: 0x80, 0x40, ...). Uvažujme následující příklad:

prefix          = 0b 0001 0010 0011 0000    // prefix = {0x12, 0x30}
prefixBitLen    = 10
prefixBitMask   = 0b 1111 1111 1100 0000    // kontrolujeme bity, kde se nachází jednička (v masce je deset jedniček zleva)

validHash       = 0b 0001 0010 0000 0101    // validní, protože se prvních deset bitů shoduje s prefixem
invalidHash     = 0b 0001 0010 1011 0000    // nevalidní, protože se 9. bit neshoduje s prefixem

## Implementační poznámky:

-   Pokud je vstup validní, pak délka pole  `prefix`  je dostatečná pro  `prefixBitLen`.