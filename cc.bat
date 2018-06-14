@echo off
rem For use with Windows
cd "nSide"

mkdir "obj"

rem Accuracy profile
mingw32-make -j4 profile=accuracy
if not exist "out\nSide.exe" (pause)

rem Balanced profile
mingw32-make -j4 profile=balanced name=nSide-balanced
if not exist "out\nSide-balanced.exe" (pause)

move "out\*.exe" ".."

cd ".."
cd "cart-pal"

mingw32-make -j4
if not exist "out\cart-pal.exe" (pause)
move "out\*.exe" ".."

@echo on