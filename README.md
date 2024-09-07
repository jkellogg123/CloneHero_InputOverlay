## Introduction
This is a Windows desktop application that will display an input overlay of your guitar inputs (frets and strums).

Press "esc" to close the input overlay, while it's in focus


## Packages
This project doesn't use any external packages besides windows.h, the SDL code which is included, and some STL headers.


## How to build
This was originally built on Windows using MinGW, g++ (13.2.0) using the following command:

***g++ -g main_qt.cpp -o main_qt.exe -ISDL2/include -ISDL2/tff/include -IQt/6.7.2/llvm-mingw_64/include -IQt/6.7.2/llvm-mingw_64/include/QtWidgets -IQt/6.7.2/llvm-mingw_64/include/QtCore -IQt/6.7.2/llvm-mingw_64/include/QtGui -LQt/6.7.2/llvm-mingw_64/lib -LSDL2/lib -LSDL2/tff/lib -lSDL2 -lSDL2_ttf -lQt6Widgets -lQt6Core -lQt6Gui -mwindows***


## New feature ideas
This section is just to put ideas for new features/changes.

1. make buttons transparent when inactive, would require gui overhaul i believe.