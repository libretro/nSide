@echo off
rem For use with Windows
mingw32-make -j4
if not exist "out/cart-pal.exe" (pause)
@echo on