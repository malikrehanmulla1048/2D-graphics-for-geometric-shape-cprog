#include <curses.h>
#include <stdio.h>

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    mouse_set(ALL_MOUSE_EVENTS);
    
    printw("Click or hover. Press 'q' to quit.\n");
    refresh();
    
    int ch;
    while((ch = getch()) != 'q') {
        if(ch == KEY_MOUSE) {
            int state = request_mouse_pos();
            mvprintw(1, 0, "Mouse at x=%d, y=%d, bstate=0x%x", Mouse_status.x, Mouse_status.y, Mouse_status.button[0]);
            refresh();
        }
    }
    
    endwin();
    return 0;
}
