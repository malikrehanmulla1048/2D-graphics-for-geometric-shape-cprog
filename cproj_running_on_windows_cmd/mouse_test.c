#include <curses.h>
#include <stdio.h>

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    
    printw("Click or hover. Press 'q' to quit.\n");
    refresh();
    
    int ch;
    MEVENT event;
    while((ch = getch()) != 'q') {
        if(ch == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                mvprintw(1, 0, "Mouse at x=%d, y=%d, bstate=0x%08lx", event.x, event.y, event.bstate);
                refresh();
            }
        }
    }
    
    endwin();
    return 0;
}
