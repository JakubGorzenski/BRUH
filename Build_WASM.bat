@echo off
cd BRUH\WASM
goto start
:loop
pause
:start
cls

clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 %1 -o a.bc -Wall -Wextra -pedantic
clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 ..\bruh.c -o b.bc -Wall -Wextra -pedantic
llvm-link a.bc b.bc -o a.bc
opt -O3 a.bc -o a.bc
llc -O3 -filetype=obj a.bc -o a.o
rem wasm-ld a.o -o a.wasm --no-entry --export-dynamic --allow-undefined 
wasm-ld a.o -o a.wasm --import-memory --strip-all --allow-undefined
rem --import-memory

7z u a.zip a.wasm index.html > nul
copy a.zip ..\..\a.zip > nul
goto loop