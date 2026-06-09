# 2D-graphics-for-geometric-shape-cprog
# 2D Graphics Editor (ASCII)

This project is a simple **Windows console-based 2D graphics editor** written in C. It draws basic shapes like circles, rectangles, lines, and triangles using ASCII characters such as `*` and `_` inside a text canvas shown in the terminal.[web:147]

The program runs inside the terminal window itself, so the drawing area is not a separate GUI window. The canvas appears as a large bordered box printed in the terminal output, above the menu options.

## Features

- Add objects: Circle, Rectangle, Line, Triangle.
- Modify existing objects using their object ID.
- Delete objects using their object ID.
- List all created objects with coordinates and parameters.
- Show an ASCII canvas whenever the main menu is displayed.
- Reset the demo scene quickly with the new `r` shortcut and clear the canvas cleanly with `c`.

## Requirements

To build and run this program on Windows, use:

- GCC for Windows, such as **MinGW** or **MSYS2 MinGW**
- PowerShell, Command Prompt, Windows Terminal, or VS Code terminal
- The source file: `graphics_editor.c`

This version uses **PDCurses** for Windows console rendering and mouse input, so the build depends on a curses-compatible library in your toolchain.

## Compile

Open PowerShell or the VS Code terminal in the folder containing `graphics_editor.c`, then run:

```powershell
gcc -Wall -Wextra graphics_editor.c -lpdcurses -o graphics_editor.exe
```

If compilation succeeds, it creates `graphics_editor.exe` in the current folder.

You can also use the included helper script:

```powershell
.\run_windows.cmd
```

Or use the simple Makefile target for a clean rebuild:

```powershell
make
```

## Run

Run the executable with:

```powershell
.\graphics_editor.exe
```

If PowerShell says the program is not recognized, make sure:

- The compile step completed successfully
- `graphics_editor.exe` exists in the current folder
- You are running it from the correct folder using `.\\graphics_editor.exe`.[web:30][web:33]

## UTF-8 / Box Character Fix

If the menu border looks garbled, such as strange symbols instead of box characters, the terminal is using the wrong code page for Unicode box-drawing characters.[web:146][web:149]

Before running the program, execute:

```powershell
chcp 65001
.\graphics_editor.exe
```

This switches the terminal to UTF-8 so box-drawing characters display more correctly on Windows consoles.[web:146][web:149]

If the border still looks messy in VS Code, replace the decorative Unicode characters in the code with plain ASCII characters like `+`, `-`, and `|`.

## How the Program Works

When the program starts, it:

1. Opens a console window
2. Creates an internal 80x40 text canvas
3. Draws demo objects on that canvas
4. Shows the canvas at the top of the terminal
5. Shows the menu below the canvas

The canvas is printed as a bordered text box similar to this:

```text
+--------------------------------------------------------------------------------+
|                                                                                |
|                     Shapes made using * and _ appear here                      |
|                                                                                |
+--------------------------------------------------------------------------------+
```

That bordered area is the **canvas**.

## Where to See the Canvas

The canvas is shown **at the top of the terminal output**, above the menu options. If you only see the menu, the terminal window may be too small, and the canvas may be above the visible area.[web:124]

### In PowerShell or Command Prompt

- Maximize the terminal window
- Run the program again
- Look above the menu options

### In VS Code

The canvas is shown in the **Integrated Terminal**, not in the code editor pane.[web:124]

To see it clearly:

1. Open the **TERMINAL** panel in VS Code
2. Maximize the terminal panel if needed
3. Run the program
4. Scroll upward if the canvas is above the visible part of the terminal output.[web:124][web:130]

If terminal history is too short, increase the VS Code scrollback setting:

- Open Settings
- Search for `terminal.integrated.scrollback`
- Set it to a larger value such as `10000`.[web:130][web:124]

You can also add this in `settings.json`:

```json
"terminal.integrated.scrollback": 10000
```

## How to Create and View Objects

Objects are visible in **two ways**:

### 1. Visually on the Canvas

Whenever the main menu is displayed, the program redraws all active objects on the canvas. That means newly created objects appear after the program returns to the main menu.

Important: while entering values in the input prompts, you are inside the input section, not inside the canvas view. The object becomes visible after the add operation finishes and the program redraws the main screen.

### 2. As a Text List

Use menu option:

```text
4. List objects
```

This prints every active object along with:

- ID
- Type
- Coordinates
- Radius (for circles)
- Character used for drawing

This is the easiest way to confirm that an object was created successfully.

## Recommended Test Object

If you are unsure whether drawing works, create a large rectangle:

- Top-left X = 5
- Top-left Y = 5
- Bottom-right X = 25
- Bottom-right Y = 15
- Character = `*`

After finishing the input, return to the main menu and look at the upper-left region of the canvas. A large rectangle should be visible there.

## Menu Overview

Main menu options:

```text
1. Add object
2. Delete object
3. Modify object
4. List objects
5. Clear all
0. Quit
```

### Add Object

Selecting `1` opens the add menu:

```text
1. Circle
2. Rectangle
3. Line
4. Triangle
0. Back
```

After entering the coordinates and character, the object is stored in memory and shown on the canvas when the main screen is redrawn.

### Delete Object

Choose `2`, then enter the object ID shown in the object list.

### Modify Object

Choose `3`, then enter the object ID. The program asks for new coordinates and updates the object.

### List Objects

Choose `4` to display all objects and their details.

### Clear All

Choose `5` to remove all objects and reset IDs.

## Coordinate System

The canvas size is:

- Width: `80`
- Height: `40`

Valid coordinates are roughly:

- `x = 0 to 79`
- `y = 0 to 39`

Anything outside these limits is ignored by the safe drawing function, so shapes can become partially or fully invisible.

## Common Problems and Fixes

### 1. Undefined Reference Errors During Compile

If you previously saw errors such as `undefined reference to start_color`, `stdscr`, `newwin`, or `wgetch`, that was caused by compiling code written for `ncurses`/`pdcurses` without linking the matching curses library.[web:144]

The current source you are using is already converted to the Windows Console API, so compile it simply with:

```powershell
gcc graphics_editor.c -o graphics_editor.exe
```

No curses library is needed for this version.

### 2. Garbled Menu Border Characters

Run:

```powershell
chcp 65001
```

before launching the program, or replace Unicode border characters with plain ASCII.[web:146][web:149]

### 3. Objects Seem Missing

Possible reasons:

- The object is above the currently visible portion of the terminal output
- The terminal window is too small
- The coordinates are outside the canvas bounds
- The shape is too small to notice
- You are still in the input prompt view instead of the main canvas view

### 4. Canvas Not Visible in VS Code

Open the **Integrated Terminal**, maximize it, and scroll upward to the top output area.[web:124][web:130]

## Code Fixes Already Identified

### 1. Demo Object Initializer Warning

The original demo object initializers had too many values for the `Object` struct, causing `excess elements in struct initializer` warnings.

Corrected examples:

```c
objects[obj_count++] = (Object){next_id++, OBJ_RECTANGLE, 2, 2, 20, 12, 0, 0, 0, '*', 1};
objects[obj_count++] = (Object){next_id++, OBJ_CIRCLE,    50, 20, 0, 0, 0, 0, 8, '_', 1};
objects[obj_count++] = (Object){next_id++, OBJ_LINE,      5, 35, 75, 5, 0, 0, 0, '*', 1};
objects[obj_count++] = (Object){next_id++, OBJ_TRIANGLE, 10, 38, 30, 22, 50, 38, 0, '_', 1};
```

### 2. `fflush(stdin)` Issue

Using `fflush(stdin)` is undefined behavior in standard C, so it should be replaced with a safer input-buffer-clearing approach.

A better alternative is to use your custom `flush()` function.

## Suggested Improvements

For better usability, consider these improvements:

- Print a label like `=== CANVAS ===` before the canvas
- Redraw the canvas immediately after adding or modifying an object
- Replace Unicode header borders with ASCII borders
- Add coordinate range validation for user input
- Show the current object count below the canvas

## Example Workflow

```powershell
cd D:\cproject
gcc graphics_editor.c -o graphics_editor.exe
chcp 65001
.\graphics_editor.exe
```

Then:

1. Choose `1` for Add object
2. Choose `2` for Rectangle
3. Enter visible coordinates such as `5`, `5`, `25`, `15`
4. Choose `*` as the character
5. Press Enter to continue
6. Return to the main menu
7. Look at the canvas above the menu
8. Use `4. List objects` to confirm the object exists

## Summary

This project is a terminal-based ASCII drawing editor for Windows. The canvas is printed inside the terminal, the menu appears below it, and objects are visible when the canvas is redrawn on the main screen.[web:147]

If the canvas is hard to see in VS Code, use the terminal panel, maximize it, and scroll upward. If the menu border is garbled, switch the console to UTF-8 with `chcp 65001`.[web:124][web:146]
