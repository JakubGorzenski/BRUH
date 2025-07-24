@echo off
goto start
:loop
pause
:start
cls

cd tmp
del ..\a.zip
echo a.wasm > manifest.txt

FOR %%i IN (%*) DO (
    IF "%%~xi"==".c" (
        clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 %%i -o a.bc -Wall -Wextra -pedantic
    ) ELSE (
        echo %%~nxi >> manifest.txt
        7z a ..\a.zip %%i > nul
    )
)

clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 ..\BRUH\bruh.c -o b.bc -Wall -Wextra -pedantic
llvm-link a.bc b.bc -o a.bc
opt -O3 a.bc -o a.bc
llc -O3 -filetype=obj a.bc -o a.o
rem wasm-ld a.o -o a.wasm --no-entry --export-dynamic --allow-undefined 
wasm-ld a.o -o a.wasm --strip-all --allow-undefined
rem --import-memory

7z a ..\a.zip ..\BRUH\WASM\index.html a.wasm manifest.txt > nul

cd ..

goto loop