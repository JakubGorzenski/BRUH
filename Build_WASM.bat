@echo off
goto start
:loop
pause
:start
cls

cd tmp
del ..\a.zip > file_list.txt

FOR %%i IN (%*) DO (
    IF "%%~xi"==".c" (
        clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 %%i -o a.bc -Wall -Wextra -pedantic
    ) ELSE (
        echo %%~nxi >> file_list.txt
        7z a ..\a.zip %%i > nul
    )
)

clang -cc1 -Ofast -emit-llvm-bc -triple=wasm32-unknown-unknown-wasm -std=c23 ..\CGL\cgl.c -o b.bc -Wall -Wextra -pedantic
llvm-link a.bc b.bc -o a.bc
opt -O3 a.bc -o a.bc
llc -O3 -filetype=obj a.bc -o a.o
rem wasm-ld a.o -o a.wasm --no-entry --export-dynamic --allow-undefined 
wasm-ld a.o -o game.wasm --strip-all --allow-undefined --export-dynamic
rem --import-memory -fno-builtins

7z a ..\a.zip ..\CGL\WASM\index.html game.wasm file_list.txt > nul

cd ..

goto loop