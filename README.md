# Dokumentace
Autor: **Marek Bitomský (xbitom00)**
Datum: **4. 3. 2022**

## Spuštění
**./hinfosvc** port

## Použití
z webového prohlížeče nebo programu, který umí odesílat HTTP požadavky pošleme **GET**
   ve formátu ***server:port/pozadavek***
| Požadavek   | Očekávaná odpověď                      | 
| ---------   | -------------------------------------- |
| `\hostname` | Doménový název počítače                |
| `\cpu-name` | Název procesoru, kde je spuštěn server |
| `\load`     | Zatížení procesoru v daný moment		 |

## Struktura 
- funkce **main()**
    - obsahuje základní nastavení socketu
- funkce **param_test()**
    - kontrolu, jestli uživatel zadal/spustil správně program a zadal port  
- funkce **get_response_from_path()**
	- generuje odpověď dle get požadavku
		- v případě */hostname*
			- pomocí funkce *gethostname()* získavám doménové jméno počítače
		- v případě */cpu-name*
    		- pomocí funkce *popen()* volám bash příkaz, který využívá:
			*lscpu* - pro zjištění informací o CPU
			*grep* - pro ořezání řádku s modelovým jménem
			*sed* - pro ořezání textu "model name:" podle regexu
		- v případě */load*
    		- pomocí funkce *popen()* podobně jako u cpu-name získávám informace
			o CPU ze souboru */proc/stat*, ale s využitím:
			*cat* - vypíše na stdout data ze souboru
			*head* - vyberu si první řádek
			*sed* - pro ořezání textu "cpu" podle regexu
			- následně pomocí funkce *strtok()* získávám tokeny - konkrétní čísla
			- z čísel ze souboru počítám dle [vzorečku](https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux) zátěž
    - u všech variant si ukládám odpověď do proměnné response, kterou
	následně odesílám zpět klientovi 

## Shrnutí 
Největší problém jsem měl s řešením zátěže, kde jsem neustále dostával jako výsledek 0%.