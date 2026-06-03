/*
 * 2D Graphics Editor for Windows Console
 * Uses Windows Console API for colors and cursor control
 * Draws using '*' and '_' characters on a 2D canvas
 *
 * Compile: gcc graphics_editor.c -o graphics_editor.exe
 * Run:     graphics_editor.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <conio.h>
#include <windows.h>

/* ─────────────────────────── CONSTANTS ─────────────────────────── */
#define CANVAS_W   80
#define CANVAS_H   40
#define MAX_OBJECTS 64
#define EMPTY      ' '
#define BORDER     '#'
#define FILL_CHAR  '*'
#define OUTLINE    '_'

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
    /* common params */
    int x1, y1;       /* origin / start */
    int x2, y2;       /* end / second point (line, rect) */
    int x3, y3;       /* third point (triangle) */
    int radius;       /* circle */
    char     ch;      /* drawing character */
    int      active;  /* 1 = exists, 0 = deleted */
} Object;

/* ─────────────────────────── GLOBALS ─────────────────────────── */
static char canvas[CANVAS_H][CANVAS_W];
static Object objects[MAX_OBJECTS];
static int obj_count   = 0;
static int next_id     = 1;
static HANDLE hConsole;

/* ─────────────────── WINDOWS CONSOLE HELPERS ─────────────────── */
static void set_color(WORD attr) {
    SetConsoleTextAttribute(hConsole, attr);
}

static void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, c);
}

static void hide_cursor(void) {
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(hConsole, &ci);
}

static void show_cursor(void) {
    CONSOLE_CURSOR_INFO ci = { 1, TRUE };
    SetConsoleCursorInfo(hConsole, &ci);
}

static void clear_screen(void) {
    system("cls");
}

/* ─────────────────────────── CANVAS OPS ─────────────────────────── */
static void canvas_clear(void) {
    for (int r = 0; r < CANVAS_H; r++)
        for (int c = 0; c < CANVAS_W; c++)
            canvas[r][c] = EMPTY;
}

/* Safe put — ignores out-of-bounds writes */
static void canvas_put(int x, int y, char ch) {
    if (x >= 0 && x < CANVAS_W && y >= 0 && y < CANVAS_H)
        canvas[y][x] = ch;
}

/* ─────────────────────── DRAWING PRIMITIVES ──────────────────────── */

/* Bresenham line */
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

/* Midpoint circle (outline) */
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

/* Rectangle (outline) */
static void draw_rectangle(int x1, int y1, int x2, int y2, char ch) {
    if (x1 > x2) { int t=x1; x1=x2; x2=t; }
    if (y1 > y2) { int t=y1; y1=y2; y2=t; }
    draw_line(x1, y1, x2, y1, ch);   /* top    */
    draw_line(x1, y2, x2, y2, ch);   /* bottom */
    draw_line(x1, y1, x1, y2, ch);   /* left   */
    draw_line(x2, y1, x2, y2, ch);   /* right  */
}

/* Triangle (3 vertices joined by lines) */
static void draw_triangle(int x1, int y1,
                           int x2, int y2,
                           int x3, int y3, char ch) {
    draw_line(x1, y1, x2, y2, ch);
    draw_line(x2, y2, x3, y3, ch);
    draw_line(x3, y3, x1, y1, ch);
}

/* ─────────────────────── RENDER ALL OBJECTS ─────────────────────── */
static void render_objects(void) {
    canvas_clear();
    for (int i = 0; i < obj_count; i++) {
        Object *o = &objects[i];
        if (!o->active) continue;
        switch (o->type) {
            case OBJ_CIRCLE:
                draw_circle(o->x1, o->y1, o->radius, o->ch); break;
            case OBJ_RECTANGLE:
                draw_rectangle(o->x1, o->y1, o->x2, o->y2, o->ch); break;
            case OBJ_LINE:
                draw_line(o->x1, o->y1, o->x2, o->y2, o->ch); break;
            case OBJ_TRIANGLE:
                draw_triangle(o->x1, o->y1,
                              o->x2, o->y2,
                              o->x3, o->y3, o->ch); break;
        }
    }
}

/* ─────────────────────────── DISPLAY ─────────────────────────── */
static void display_canvas(void) {
    clear_screen();
    /* Top border */
    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("+");
    for (int c = 0; c < CANVAS_W; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < CANVAS_H; r++) {
        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("|");
        for (int c = 0; c < CANVAS_W; c++) {
            char ch = canvas[r][c];
            if (ch == '*') {
                set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            } else if (ch == '_') {
                set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            } else {
                set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
            putchar(ch);
        }
        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("|\n");
    }

    /* Bottom border */
    printf("+");
    for (int c = 0; c < CANVAS_W; c++) printf("-");
    printf("+\n");
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/* ─────────────────────────── OBJECT LIST ─────────────────────────── */
static void list_objects(void) {
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("\n  %-4s %-12s %s\n", "ID", "Type", "Parameters");
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("  %-4s %-12s %s\n", "--", "----", "----------");

    int found = 0;
    for (int i = 0; i < obj_count; i++) {
        Object *o = &objects[i];
        if (!o->active) continue;
        found = 1;
        const char *tname = "?";
        switch (o->type) {
            case OBJ_CIRCLE:    tname = "Circle";    break;
            case OBJ_RECTANGLE: tname = "Rectangle"; break;
            case OBJ_LINE:      tname = "Line";      break;
            case OBJ_TRIANGLE:  tname = "Triangle";  break;
        }
        printf("  %-4d %-12s ", o->id, tname);
        switch (o->type) {
            case OBJ_CIRCLE:
                printf("cx=%d cy=%d r=%d ch='%c'",
                        o->x1, o->y1, o->radius, o->ch); break;
            case OBJ_RECTANGLE:
                printf("(%d,%d)->(%d,%d) ch='%c'",
                        o->x1, o->y1, o->x2, o->y2, o->ch); break;
            case OBJ_LINE:
                printf("(%d,%d)->(%d,%d) ch='%c'",
                        o->x1, o->y1, o->x2, o->y2, o->ch); break;
            case OBJ_TRIANGLE:
                printf("(%d,%d) (%d,%d) (%d,%d) ch='%c'",
                        o->x1, o->y1, o->x2, o->y2,
                        o->x3, o->y3, o->ch); break;
        }
        putchar('\n');
    }
    if (!found) printf("  (no objects)\n");
}

/* ─────────────────────────── INPUT HELPERS ─────────────────────────── */
static int read_int(const char *prompt) {
    int v;
    printf("  %s: ", prompt);
    while (scanf("%d", &v) != 1) {
        printf("  Invalid – enter an integer: ");
        while (getchar() != '\n');
    }
    return v;
}

static char read_char(const char *prompt) {
    char buf[8];
    printf("  %s [* or _] (default *): ", prompt);
    fflush(stdin);
    fgets(buf, sizeof(buf), stdin);
    /* skip leftover newline from previous scanf */
    if (buf[0] == '\n') {
        fgets(buf, sizeof(buf), stdin);
    }
    return (buf[0] == '_') ? '_' : '*';
}

/* Flush stdin after scanf */
static void flush(void) { while (getchar() != '\n'); }

/* ─────────────────── FIND OBJECT BY ID ─────────────────── */
static Object *find_object(int id) {
    for (int i = 0; i < obj_count; i++)
        if (objects[i].id == id && objects[i].active)
            return &objects[i];
    return NULL;
}

/* ─────────────────── ADD OPERATIONS ─────────────────── */
static void add_circle(void) {
    if (obj_count >= MAX_OBJECTS) { printf("  Canvas full!\n"); return; }
    Object *o = &objects[obj_count++];
    o->id     = next_id++;
    o->type   = OBJ_CIRCLE;
    o->active = 1;
    o->x1     = read_int("Center X (0-79)");
    o->y1     = read_int("Center Y (0-39)");
    o->radius = read_int("Radius");
    flush();
    o->ch     = read_char("Character");
    printf("  Circle #%d added.\n", o->id);
}

static void add_rectangle(void) {
    if (obj_count >= MAX_OBJECTS) { printf("  Canvas full!\n"); return; }
    Object *o = &objects[obj_count++];
    o->id     = next_id++;
    o->type   = OBJ_RECTANGLE;
    o->active = 1;
    o->x1     = read_int("Top-left X");
    o->y1     = read_int("Top-left Y");
    o->x2     = read_int("Bottom-right X");
    o->y2     = read_int("Bottom-right Y");
    flush();
    o->ch     = read_char("Character");
    printf("  Rectangle #%d added.\n", o->id);
}

static void add_line(void) {
    if (obj_count >= MAX_OBJECTS) { printf("  Canvas full!\n"); return; }
    Object *o = &objects[obj_count++];
    o->id     = next_id++;
    o->type   = OBJ_LINE;
    o->active = 1;
    o->x1     = read_int("Start X");
    o->y1     = read_int("Start Y");
    o->x2     = read_int("End X");
    o->y2     = read_int("End Y");
    flush();
    o->ch     = read_char("Character");
    printf("  Line #%d added.\n", o->id);
}

static void add_triangle(void) {
    if (obj_count >= MAX_OBJECTS) { printf("  Canvas full!\n"); return; }
    Object *o = &objects[obj_count++];
    o->id     = next_id++;
    o->type   = OBJ_TRIANGLE;
    o->active = 1;
    o->x1     = read_int("Vertex 1 X");
    o->y1     = read_int("Vertex 1 Y");
    o->x2     = read_int("Vertex 2 X");
    o->y2     = read_int("Vertex 2 Y");
    o->x3     = read_int("Vertex 3 X");
    o->y3     = read_int("Vertex 3 Y");
    flush();
    o->ch     = read_char("Character");
    printf("  Triangle #%d added.\n", o->id);
}

/* ─────────────────── DELETE OPERATION ─────────────────── */
static void delete_object(void) {
    list_objects();
    int id = read_int("\nEnter ID to delete (0=cancel)");
    if (id == 0) return;
    Object *o = find_object(id);
    if (!o) { printf("  ID %d not found.\n", id); return; }
    o->active = 0;
    printf("  Object #%d deleted.\n", id);
    flush();
}

/* ─────────────────── MODIFY OPERATION ─────────────────── */
static void modify_object(void) {
    list_objects();
    int id = read_int("\nEnter ID to modify (0=cancel)");
    if (id == 0) { flush(); return; }
    Object *o = find_object(id);
    if (!o) { printf("  ID %d not found.\n", id); flush(); return; }
    flush();

    printf("\n  Modifying object #%d – enter new values:\n", id);
    switch (o->type) {
        case OBJ_CIRCLE:
            o->x1     = read_int("New Center X");
            o->y1     = read_int("New Center Y");
            o->radius = read_int("New Radius");
            flush();
            o->ch     = read_char("New Character");
            break;
        case OBJ_RECTANGLE:
            o->x1 = read_int("New Top-left X");
            o->y1 = read_int("New Top-left Y");
            o->x2 = read_int("New Bottom-right X");
            o->y2 = read_int("New Bottom-right Y");
            flush();
            o->ch = read_char("New Character");
            break;
        case OBJ_LINE:
            o->x1 = read_int("New Start X");
            o->y1 = read_int("New Start Y");
            o->x2 = read_int("New End X");
            o->y2 = read_int("New End Y");
            flush();
            o->ch = read_char("New Character");
            break;
        case OBJ_TRIANGLE:
            o->x1 = read_int("New Vertex 1 X");
            o->y1 = read_int("New Vertex 1 Y");
            o->x2 = read_int("New Vertex 2 X");
            o->y2 = read_int("New Vertex 2 Y");
            o->x3 = read_int("New Vertex 3 X");
            o->y3 = read_int("New Vertex 3 Y");
            flush();
            o->ch = read_char("New Character");
            break;
    }
    printf("  Object #%d updated.\n", id);
}

/* ─────────────────────────── MENUS ─────────────────────────── */
static void print_header(void) {
    set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║     2D GRAPHICS EDITOR  (ASCII)  ║\n");
    printf("  ╚══════════════════════════════════╝\n");
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

static void add_menu(void) {
    while (1) {
        clear_screen();
        print_header();
        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("\n  ── Add Object ──\n\n");
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("  1. Circle\n");
        printf("  2. Rectangle\n");
        printf("  3. Line\n");
        printf("  4. Triangle\n");
        printf("  0. Back\n\n");

        int ch = read_int("Choice");
        flush();
        switch (ch) {
            case 1: add_circle();    break;
            case 2: add_rectangle(); break;
            case 3: add_line();      break;
            case 4: add_triangle();  break;
            case 0: return;
            default: printf("  Invalid choice.\n");
        }
        printf("\n  Press ENTER to continue...");
        getchar();
    }
}

static void main_menu(void) {
    while (1) {
        render_objects();
        display_canvas();

        set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("\n  ╔══════════════════════════════════╗\n");
        printf("  ║     2D GRAPHICS EDITOR  (ASCII)  ║\n");
        printf("  ╚══════════════════════════════════╝\n");
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\n  1. Add object\n");
        printf("  2. Delete object\n");
        printf("  3. Modify object\n");
        printf("  4. List objects\n");
        printf("  5. Clear all\n");
        printf("  0. Quit\n\n");

        int choice = read_int("Choice");
        flush();

        switch (choice) {
            case 1:
                add_menu();
                break;
            case 2:
                clear_screen();
                print_header();
                delete_object();
                printf("\n  Press ENTER to continue...");
                getchar();
                break;
            case 3:
                clear_screen();
                print_header();
                modify_object();
                printf("\n  Press ENTER to continue...");
                getchar();
                break;
            case 4:
                clear_screen();
                print_header();
                list_objects();
                printf("\n  Press ENTER to continue...");
                getchar();
                break;
            case 5:
                obj_count = 0;
                next_id   = 1;
                canvas_clear();
                printf("  Canvas cleared.\n");
                Sleep(600);
                break;
            case 0:
                clear_screen();
                set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                printf("  Goodbye!\n\n");
                show_cursor();
                exit(0);
            default:
                printf("  Invalid choice.\n");
                Sleep(400);
        }
    }
}

/* ─────────────────────────── MAIN ─────────────────────────── */
int main(void) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Make the console window large enough */
    SMALL_RECT sr = { 0, 0, 100, 55 };
    SetConsoleWindowInfo(hConsole, TRUE, &sr);
    COORD cs = { 101, 56 };
    SetConsoleScreenBufferSize(hConsole, cs);

    hide_cursor();
    canvas_clear();

    /* Seed with a demo scene */
    objects[obj_count++] = (Object){next_id++, OBJ_RECTANGLE, 2, 2, 20, 12, 0, 0, 0, '*', 1};
    objects[obj_count++] = (Object){next_id++, OBJ_CIRCLE,    50, 20, 0,  0, 0, 0, 8, '_', 1};
    objects[obj_count++] = (Object){next_id++, OBJ_LINE,       5, 35, 75, 5, 0, 0, 0, '*', 1};
    objects[obj_count++] = (Object){next_id++, OBJ_TRIANGLE,  10, 38, 30, 22, 50, 38, 0, '_', 1};

    main_menu();
    return 0;
}
