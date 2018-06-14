@echo off
rem For use with Windows.
rem This script compiles executables with both the accuracy and balanced
rem profiles, as well as a libretro core of the balanced profile.

rem Accuracy profile
mingw32-make -j6
if exist "out\nSide.exe" (
  rem Balanced profile
  mingw32-make -j6 profile=balanced name=nSide-balanced
  if exist "out\nSide-balanced.exe" (
    mingw32-make -j6 binary=library target=libretro core=sfc-balanced
    if not exist "out\higan_sfc_balanced_libretro.dll" (pause)
  ) else (
    pause
  )
) else (
  pause
)

@echo on