all: graphics_editor.exe

graphics_editor.exe: graphics_editor.c
	gcc -Wall -Wextra -O2 graphics_editor.c -lpdcurses -o graphics_editor.exe

clean:
	del /Q graphics_editor.exe 2>nul
