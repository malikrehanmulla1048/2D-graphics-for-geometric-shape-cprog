/*
 * 2D Graphics Editor for Windows Console
 * Uses PDCurses for UI and Mouse interaction
 * Draws using '*' and '_' characters on a 2D canvas
 *
 * Compile: gcc graphics_editor.c -lpdcurses -o graphics_editor.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curses.h>

/* ─────────────────────────── CONSTANTS ─────────────────────────── */
#define CANVAS_W   80
#define CANVAS_H   40
#define MENU_W     20
#define MAX_OBJECTS 128
#define EMPTY      ' '

/* ─────────────────────────── TYPES ─────────────────────────── */
typedef enum {
    OBJ_CIRCLE = 1,
    OBJ_RECTANGLE,
    OBJ_LINE,
    OBJ_TRIANGLE
} ObjType;

typedef struct {
    int      id;
    ObjType  type;
    int x1, y1;       /* origin / start */
    int x2, y2;       /* end / second point (line, rect) */
    int x3, y3;       /* third point (triangle) */
    int radius;       /* circle */
    char     ch;      /* drawing character */
    int      active;  /* 1 = exists, 0 = deleted */
} Object;

typedef enum {
    TOOL_NONE,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_CIRCLE,
    TOOL_TRIANGLE,
    TOOL_DELETE,
    TOOL_MODIFY
} Tool;

/* ─────────────────────────── GLOBALS ─────────────────────────── */
static char canvas[CANVAS_H][CANVAS_W];
static Object objects[MAX_OBJECTS];
static int obj_count   = 0;
static int next_id     = 1;
static float zoom_factor = 1.0f;

static Tool current_tool = TOOL_NONE;
static int step = 0; /* Interaction step for multi-click tools */

/* Temporary points for drawing */
static int px1, py1, px2, py2;

/* ─────────────────────────── CANVAS OPS ─────────────────────────── */
static void canvas_clear(void) {
    for (int r = 0; r < CANVAS_H; r++)
        for (int c = 0; c < CANVAS_W; c++)
            canvas[r][c] = EMPTY;
}

static void canvas_put(int x, int y, char ch) {
    if (x >= 0 && x < CANVAS_W && y >= 0 && y < CANVAS_H)
        canvas[y][x] = ch;
}

/* ─────────────────────── DRAWING PRIMITIVES ──────────────────────── */
static void draw_line(int x0, int y0, int x1, int y1, char ch) {
    int dx  = abs(x1 - x0), dy = abs(y1 - y0);
    int sx  = x0 < x1 ? 1 : -1;
    int sy  = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2;
    for (;;) {
        canvas_put(x0, y0, ch);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

static void draw_circle(int cx, int cy, int r, char ch) {
    int x = 0, y = r, d = 3 - 2 * r;
    while (x <= y) {
        canvas_put(cx+x, cy+y, ch); canvas_put(cx-x, cy+y, ch);
        canvas_put(cx+x, cy-y, ch); canvas_put(cx-x, cy-y, ch);
        canvas_put(cx+y, cy+x, ch); canvas_put(cx-y, cy+x, ch);
        canvas_put(cx+y, cy-x, ch); canvas_put(cx-y, cy-x, ch);
        if (d < 0) d += 4*x + 6;
        else { d += 4*(x-y) + 10; y--; }
        x++;
    }
}

static void draw_rectangle(int x1, int y1, int x2, int y2, char ch) {
    if (x1 > x2) { int t=x1; x1=x2; x2=t; }
    if (y1 > y2) { int t=y1; y1=y2; y2=t; }
    draw_line(x1, y1, x2, y1, ch);   /* top    */
    draw_line(x1, y2, x2, y2, ch);   /* bottom */
    draw_line(x1, y1, x1, y2, ch);   /* left   */
    draw_line(x2, y1, x2, y2, ch);   /* right  */
}

static void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    draw_line(x1, y1, x2, y2, ch);
    draw_line(x2, y2, x3, y3, ch);
    draw_line(x3, y3, x1, y1, ch);
}

/* ─────────────────────── RENDER ALL OBJECTS ─────────────────────── */
static void render_objects(void) {
    canvas_clear();
    int cx = CANVAS_W / 2;
    int cy = CANVAS_H / 2;
    for (int i = 0; i < obj_count; i++) {
        Object *o = &objects[i];
        if (!o->active) continue;
        
        int sx1 = cx + (int)round((o->x1 - cx) * zoom_factor);
        int sy1 = cy + (int)round((o->y1 - cy) * zoom_factor);
        int sx2 = cx + (int)round((o->x2 - cx) * zoom_factor);
        int sy2 = cy + (int)round((o->y2 - cy) * zoom_factor);
        int sx3 = cx + (int)round((o->x3 - cx) * zoom_factor);
        int sy3 = cy + (int)round((o->y3 - cy) * zoom_factor);
        int srad = (int)round(o->radius * zoom_factor);

        switch (o->type) {
            case OBJ_CIRCLE:
                draw_circle(sx1, sy1, srad, o->ch); break;
            case OBJ_RECTANGLE:
                draw_rectangle(sx1, sy1, sx2, sy2, o->ch); break;
            case OBJ_LINE:
                draw_line(sx1, sy1, sx2, sy2, o->ch); break;
            case OBJ_TRIANGLE:
                draw_triangle(sx1, sy1, sx2, sy2, sx3, sy3, o->ch); break;
        }
    }
}

/* Add a new object */
static void add_object(ObjType type, int x1, int y1, int x2, int y2, int x3, int y3, int r, char ch) {
    if (obj_count >= MAX_OBJECTS) return;
    Object *o = &objects[obj_count++];
    o->id     = next_id++;
    o->type   = type;
    o->active = 1;
    o->x1 = x1; o->y1 = y1;
    o->x2 = x2; o->y2 = y2;
    o->x3 = x3; o->y3 = y3;
    o->radius = r;
    o->ch = ch;
}

/* Simple distance helper */
static float point_dist(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/* Check if point is near a line segment */
static int is_near_line(int px, int py, int x1, int y1, int x2, int y2) {
    float d1 = point_dist(px, py, x1, y1);
    float d2 = point_dist(px, py, x2, y2);
    float len = point_dist(x1, y1, x2, y2);
    if (d1 + d2 >= len - 1.0f && d1 + d2 <= len + 1.0f) return 1;
    return 0;
}

/* Find object near point (for click selection) */
static Object* find_object_at(int px, int py) {
    int cx = CANVAS_W / 2;
    int cy = CANVAS_H / 2;
    for (int i = obj_count - 1; i >= 0; i--) {
        Object *o = &objects[i];
        if (!o->active) continue;
        
        int sx1 = cx + (int)round((o->x1 - cx) * zoom_factor);
        int sy1 = cy + (int)round((o->y1 - cy) * zoom_factor);
        int sx2 = cx + (int)round((o->x2 - cx) * zoom_factor);
        int sy2 = cy + (int)round((o->y2 - cy) * zoom_factor);
        int sx3 = cx + (int)round((o->x3 - cx) * zoom_factor);
        int sy3 = cy + (int)round((o->y3 - cy) * zoom_factor);
        int srad = (int)round(o->radius * zoom_factor);
        
        if (o->type == OBJ_CIRCLE) {
            float d = point_dist(px, py, sx1, sy1);
            if (fabs(d - srad) <= 2.0f) return o;
        } else if (o->type == OBJ_LINE) {
            if (is_near_line(px, py, sx1, sy1, sx2, sy2)) return o;
        } else if (o->type == OBJ_RECTANGLE) {
            if (is_near_line(px, py, sx1, sy1, sx2, sy1) || is_near_line(px, py, sx1, sy2, sx2, sy2) ||
                is_near_line(px, py, sx1, sy1, sx1, sy2) || is_near_line(px, py, sx2, sy1, sx2, sy2))
                return o;
        } else if (o->type == OBJ_TRIANGLE) {
            if (is_near_line(px, py, sx1, sy1, sx2, sy2) || is_near_line(px, py, sx2, sy2, sx3, sy3) ||
                is_near_line(px, py, sx3, sy3, sx1, sy1))
                return o;
        }
    }
    return NULL;
}

/* ─────────────────────────── UI RENDERING ─────────────────────────── */
static void draw_menu(WINDOW *win) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 4, "TOOLS MENU");
    mvwprintw(win, 2, 1, "------------------");
    
    const char *tools[] = {
        "1: Line", "2: Rectangle", "3: Circle", "4: Triangle", 
        "5: Delete", "6: Modify Char"
    };
    
    for (int i = 0; i < 6; i++) {
        if (current_tool == i + 1) wattron(win, A_REVERSE);
        mvwprintw(win, 4 + i*2, 2, "%s", tools[i]);
        if (current_tool == i + 1) wattroff(win, A_REVERSE);
    }
    
    mvwprintw(win, 17, 1, "------------------");
    mvwprintw(win, 19, 2, "+: Zoom In");
    mvwprintw(win, 21, 2, "-: Zoom Out");
    mvwprintw(win, 23, 2, "c: Clear Canvas");
    mvwprintw(win, 25, 2, "q: Quit");
    
    mvwprintw(win, 28, 1, "Zoom: %d%%", (int)(zoom_factor * 100));
    
    mvwprintw(win, 30, 1, "Status:");
    switch(current_tool) {
        case TOOL_NONE: mvwprintw(win, 31, 2, "Idle"); break;
        case TOOL_LINE: mvwprintw(win, 31, 2, "Click P%d", step+1); break;
        case TOOL_RECT: mvwprintw(win, 31, 2, "Click P%d", step+1); break;
        case TOOL_CIRCLE: mvwprintw(win, 31, 2, step==0?"Center":"Radius"); break;
        case TOOL_TRIANGLE: mvwprintw(win, 31, 2, "Click P%d", step+1); break;
        case TOOL_DELETE: mvwprintw(win, 31, 2, "Click Object"); break;
        case TOOL_MODIFY: mvwprintw(win, 31, 2, "Click to toggle"); break;
    }
    
    wrefresh(win);
}

static void get_base_coord(int zx, int zy, int *bx, int *by);

static void draw_canvas(WINDOW *win, int mx, int my) {
    werase(win);
    render_objects();
    
    /* Draw rubber-band preview if in middle of operation */
    if (current_tool != TOOL_NONE && step > 0) {
        int cx = mx - 1; /* Adjust for border */
        int cy = my - 1;
        
        int center_x = CANVAS_W / 2;
        int center_y = CANVAS_H / 2;
        int spx1 = center_x + (int)round((px1 - center_x) * zoom_factor);
        int spy1 = center_y + (int)round((py1 - center_y) * zoom_factor);
        int spx2 = center_x + (int)round((px2 - center_x) * zoom_factor);
        int spy2 = center_y + (int)round((py2 - center_y) * zoom_factor);
        
        if (current_tool == TOOL_LINE && step == 1) {
            draw_line(spx1, spy1, cx, cy, '+');
        } else if (current_tool == TOOL_RECT && step == 1) {
            draw_rectangle(spx1, spy1, cx, cy, '+');
        } else if (current_tool == TOOL_CIRCLE && step == 1) {
            int bx, by;
            get_base_coord(cx, cy, &bx, &by);
            int base_r = (int)round(point_dist(px1, py1, bx, by));
            int sr = (int)round(base_r * zoom_factor);
            draw_circle(spx1, spy1, sr, '+');
        } else if (current_tool == TOOL_TRIANGLE) {
            if (step == 1) {
                draw_line(spx1, spy1, cx, cy, '+');
            } else if (step == 2) {
                draw_line(spx1, spy1, spx2, spy2, '*');
                draw_line(spx2, spy2, cx, cy, '+');
                draw_line(cx, cy, spx1, spy1, '+');
            }
        }
    }
    
    /* Blit 2D array to ncurses window */
    for (int r = 0; r < CANVAS_H; r++) {
        for (int c = 0; c < CANVAS_W; c++) {
            char ch = canvas[r][c];
            if (ch != EMPTY) {
                if (ch == '*') wattron(win, COLOR_PAIR(1) | A_BOLD);
                else if (ch == '_') wattron(win, COLOR_PAIR(2) | A_BOLD);
                else wattron(win, COLOR_PAIR(3));
                
                mvwaddch(win, r + 1, c + 1, ch);
                
                wattroff(win, COLOR_PAIR(1) | A_BOLD);
                wattroff(win, COLOR_PAIR(2) | A_BOLD);
                wattroff(win, COLOR_PAIR(3));
            }
        }
    }
    box(win, 0, 0);
    wrefresh(win);
}

/* Adjust un-zoomed coordinate to base coordinate */
static void get_base_coord(int zx, int zy, int *bx, int *by) {
    int cx = CANVAS_W / 2;
    int cy = CANVAS_H / 2;
    *bx = cx + (int)round((zx - cx) / zoom_factor);
    *by = cy + (int)round((zy - cy) / zoom_factor);
}

/* ─────────────────────────── MAIN ─────────────────────────── */
int main(void) {
    system("mode con: cols=105 lines=45");
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
    mouse_set(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION);
    
    WINDOW *menu_win = newwin(CANVAS_H + 2, MENU_W, 0, 0);
    WINDOW *canvas_win = newwin(CANVAS_H + 2, CANVAS_W + 2, 0, MENU_W);
    
    /* Seed with demo */
    add_object(OBJ_RECTANGLE, 2, 2, 20, 12, 0, 0, 0, '*');
    add_object(OBJ_CIRCLE, 50, 20, 0, 0, 0, 0, 8, '_');
    
    int mx = 0, my = 0;
    int running = 1;
    
    while (running) {
        draw_menu(menu_win);
        draw_canvas(canvas_win, mx, my);
        
        int ch = getch();
        if (ch == ERR) {
            napms(16); /* ~60fps */
            continue;
        }
        
        if (ch == 'q' || ch == 'Q') running = 0;
        else if (ch == '+') zoom_factor += 0.25f;
        else if (ch == '-') { if (zoom_factor > 0.25f) zoom_factor -= 0.25f; }
        else if (ch == 'c' || ch == 'C') { obj_count = 0; step = 0; }
        else if (ch >= '1' && ch <= '6') {
            current_tool = (Tool)(ch - '0');
            step = 0;
        }
        else if (ch == KEY_MOUSE) {
            if (request_mouse_pos() == OK) {
                /* Map global mouse coords to window */
                int gx = Mouse_status.x;
                int gy = Mouse_status.y;
                
                if (Mouse_status.changes & MOUSE_MOVED) {
                    /* Hovering updates mx, my if inside canvas */
                    if (gx > MENU_W && gx <= MENU_W + CANVAS_W && gy > 0 && gy <= CANVAS_H) {
                        mx = gx - MENU_W;
                        my = gy;
                    }
                } else if (Mouse_status.button[0] & BUTTON_PRESSED || Mouse_status.button[0] & BUTTON_CLICKED) {
                    /* Left Click */
                    if (gx < MENU_W) {
                        /* Clicked on menu area */
                        int row = gy;
                        if (row == 4) current_tool = TOOL_LINE;
                        else if (row == 6) current_tool = TOOL_RECT;
                        else if (row == 8) current_tool = TOOL_CIRCLE;
                        else if (row == 10) current_tool = TOOL_TRIANGLE;
                        else if (row == 12) current_tool = TOOL_DELETE;
                        else if (row == 14) current_tool = TOOL_MODIFY;
                        step = 0;
                    } else if (gx > MENU_W && gx <= MENU_W + CANVAS_W && gy > 0 && gy <= CANVAS_H) {
                        /* Clicked on Canvas */
                        int cx = gx - MENU_W - 1; /* 0-based canvas coords */
                        int cy = gy - 1;
                        int bx, by;
                        get_base_coord(cx, cy, &bx, &by);
                        
                        if (current_tool == TOOL_LINE) {
                            if (step == 0) { px1 = bx; py1 = by; step = 1; }
                            else { add_object(OBJ_LINE, px1, py1, bx, by, 0, 0, 0, '*'); step = 0; }
                        } else if (current_tool == TOOL_RECT) {
                            if (step == 0) { px1 = bx; py1 = by; step = 1; }
                            else { add_object(OBJ_RECTANGLE, px1, py1, bx, by, 0, 0, 0, '*'); step = 0; }
                        } else if (current_tool == TOOL_CIRCLE) {
                            if (step == 0) { px1 = bx; py1 = by; step = 1; }
                            else {
                                int r = (int)round(point_dist(px1, py1, bx, by));
                                add_object(OBJ_CIRCLE, px1, py1, 0, 0, 0, 0, r, '*');
                                step = 0;
                            }
                        } else if (current_tool == TOOL_TRIANGLE) {
                            if (step == 0) { px1 = bx; py1 = by; step = 1; }
                            else if (step == 1) { px2 = bx; py2 = by; step = 2; }
                            else { add_object(OBJ_TRIANGLE, px1, py1, px2, py2, bx, by, 0, '*'); step = 0; }
                        } else if (current_tool == TOOL_DELETE) {
                            Object *o = find_object_at(cx, cy);
                            if (o) o->active = 0;
                        } else if (current_tool == TOOL_MODIFY) {
                            Object *o = find_object_at(cx, cy);
                            if (o) {
                                o->ch = (o->ch == '*') ? '_' : (o->ch == '_' ? '#' : '*');
                            }
                        }
                    }
                }
            }
        }
    }
    
    endwin();
    return 0;
}
