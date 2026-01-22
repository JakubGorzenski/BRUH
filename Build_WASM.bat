@echo off
goto start
:loop
pause
:start
cls

cd tmp
rmdir /S /Q ROM
mkdir ROM
del ..\a.zip

FOR %%i IN (%*) DO (
    IF "%%~xi"==".c" (
        clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 %%i -o a.bc -Wall -Wextra -pedantic
    ) ELSE (
        copy %%i ROM\%%~nxi > nul
    )
)

clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 ..\BRUH\bruh.c -o b.bc -Wall -Wextra -pedantic
llvm-link a.bc b.bc -o a.bc
opt -O3 a.bc -o a.bc
llc -O3 -filetype=obj a.bc -o a.o
rem wasm-ld a.o -o a.wasm --no-entry --export-dynamic --allow-undefined 
wasm-ld a.o -o main.wasm --strip-all --allow-undefined --export-dynamic
rem --import-memory -fno-builtins
copy .\main.wasm     ROM\main.wasm > nul

7z a ..\a.zip ..\BRUH\index.html ROM > nul

cd ..

goto loop