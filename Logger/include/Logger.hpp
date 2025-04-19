#pragma once

/* Library we are going to use to log outputs : spdlog*/

/*
Logowania dotyczące : skanowania pliku , skanowania katalogu czy poszly dobrze czy cos sie nie udalo, status skanowania
szybkiego i calkowitego , czyli ile wirusow wykrylo , czy poszlo dobrze czy napotaklo problem , ilosc wykrytych wirusuow
w katalogu/ pliku, log zmiany czasu pomiedzy automatycznymi skanami, wyniki clearpwania pliku/katalogu/komptera, czy sie
clearowania udalo czy nie, ile wirusow oczyszczono, log z update info na temat plikow i katalogow i ten log pokazuje
rowniez gdzie zapisano te updaty i o ktorej je wykonano, log update katalogu, wyniki clearowania komputera z smieci ,
czyli sie sie powiiodlo czy nie , i ile udalo sie zwolnic miejsca z tych smieci 
*/

/*
DEBUG
Logowania uruchomienia antywirusa
Poziom logowania (error, success, warn ...)
Dev debug logs -> Skanowanie pliku ... , stan zagrozenia ...

Kompatkowy raport z automatycznego skanowania i usuwania garbage 

[2025.04.19] QUICK SCAN : 1200 FILES HAVE BEEN SCANNED, 8 THREATS FOUND.
[2025.04.19] GARBAGE COLLECTING : FREED 513 MB.
*/

class Logger {};
