@echo off
goto start
:loop
pause
:start
cls
clang.exe -fuse-ld=lld.exe -O3 -o a.exe %1 bruh/bruh.c -std=c23 -Wall -Wextra -pedantic -lole32 -lwinmm -lgdi32 -lksuser
rem -mwindows
a.exe
goto loop