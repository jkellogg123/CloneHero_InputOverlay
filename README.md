## Introduction
This is a Windows desktop application that will display an input overlay of your guitar inputs (frets and strums).

Press "esc" to close the input overlay, while it's in focus
Can click and hold to drag, then right-click to return overlay to bottom-right of screen


## Packages
1. Qt installation (Core, Widgets, Gui - dlls are included)
2. SDL (included)
3. windows.h and STL


## How to build
This was originally built on Windows using MinGW, g++ (13.2.0) using the following command:

***g++ -g main_qt.cpp -o main_qt.exe -ISDL2/include -ISDL2/tff/include -IQt/6.7.2/llvm-mingw_64/include -IQt/6.7.2/llvm-mingw_64/include/QtWidgets -IQt/6.7.2/llvm-mingw_64/include/QtCore -IQt/6.7.2/llvm-mingw_64/include/QtGui -LQt/6.7.2/llvm-mingw_64/lib -LSDL2/lib -LSDL2/tff/lib -lSDL2 -lSDL2_ttf -lQt6Widgets -lQt6Core -lQt6Gui -mwindows***


## New feature ideas
This section is just to put ideas for new features/changes.

1. manage the metric ton of dlls (platforms directory too) better / organize repo better lol