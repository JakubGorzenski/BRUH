@echo off
goto start
:loop
pause
:start
cls
clang.exe -fuse-ld=lld.exe -g -gcodeview -Wl,/debug,/pdb:a.pdb -o a.exe %1 BRUH/0.2/bruh.c -std=c23 -Wall -Wextra -pedantic -lole32 -lwinmm -lgdi32 -lksuser
rem -mwindows
a.exe
goto loop