@echo off
:: Build script for MinGW (gcc.exe) from Raylib distribution
:: ensure that raylib is located at C:\raylib\raylib
:: w64devkit is located at C:\raylib\w64devkit

set PATH=C:\raylib\w64devkit\bin;%PATH%
set RAYLIB_PATH=C:\raylib\raylib
set INCLUDE_PATH=-I"%RAYLIB_PATH%\src" -I.
set LIB_PATH=-L"%RAYLIB_PATH%\src"

gcc -o graph_calc.exe main.c parser.c graph.c ui.c %INCLUDE_PATH% %LIB_PATH% -lraylib -lopengl32 -lgdi32 -lwinmm
