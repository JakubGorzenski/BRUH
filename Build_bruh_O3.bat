@echo off
goto start
:loop
pause
:start
cls

clang.exe -flto -march=native -fuse-ld=lld -O3 -o a.exe %1 CGL/cgl.c -std=c23 -Wall -Wextra -pedantic -lole32 -lwinmm -lgdi32 -lksuser
rem -mwindows
a.exe

goto loop