#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH  80
#define HEIGHT 24
#define EMPTY  '_'
#define PIXEL  '*'
#define MAX_OBJECTS 100

char picture[HEIGHT][WIDTH];

/* ── canvas helpers ─────────────────────────────────────────────────── */

void clearPicture() {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            picture[y][x] = EMPTY;
}

void displayPicture() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++)
            putchar(picture[y][x]);
        putchar('\n');
    }
}

void setPixel(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        picture[y][x] = PIXEL;
}

/* ── drawing primitives ─────────────────────────────────────────────── */

void drawLine(int x1, int y1, int x2, int y2) {
    /* Bresenham's line algorithm */
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        setPixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2) {
    drawLine(x1, y1, x2, y1); /* top    */
    drawLine(x1, y2, x2, y2); /* bottom */
    drawLine(x1, y1, x1, y2); /* left   */
    drawLine(x2, y1, x2, y2); /* right  */
}

void drawCircle(int cx, int cy, int radius) {
    /* Midpoint circle algorithm */
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        setPixel(cx + x, cy + y);
        setPixel(cx - x, cy + y);
        setPixel(cx + x, cy - y);
        setPixel(cx - x, cy - y);
        setPixel(cx + y, cy + x);
        setPixel(cx - y, cy + x);
        setPixel(cx + y, cy - x);
        setPixel(cx - y, cy - x);

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);
}

/* ── object store ───────────────────────────────────────────────────── */

typedef enum { SHAPE_LINE = 1, SHAPE_RECT, SHAPE_CIRCLE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int params[6]; /* up to 6 ints depending on shape */
} Object;

Object objects[MAX_OBJECTS];
int objectCount = 0;

void redrawAll() {
    clearPicture();
    for (int i = 0; i < objectCount; i++) {
        Object *o = &objects[i];
        switch (o->type) {
            case SHAPE_LINE:
                drawLine(o->params[0], o->params[1],
                         o->params[2], o->params[3]);
                break;
            case SHAPE_RECT:
                drawRectangle(o->params[0], o->params[1],
                              o->params[2], o->params[3]);
                break;
            case SHAPE_CIRCLE:
                drawCircle(o->params[0], o->params[1], o->params[2]);
                break;
            case SHAPE_TRIANGLE:
                drawTriangle(o->params[0], o->params[1],
                             o->params[2], o->params[3],
                             o->params[4], o->params[5]);
                break;
        }
    }
}

/* ── menu helpers ───────────────────────────────────────────────────── */

void printMainMenu() {
    printf("\n2D Graphics Editor\n");
    printf("Canvas size: %d x %d\n", WIDTH, HEIGHT);
    printf("1. Add object\n");
    printf("2. Delete object\n");
    printf("3. Modify object\n");
    printf("4. Display picture\n");
    printf("5. List objects\n");
    printf("0. Exit\n");
    printf("Enter choice: ");
}

void printShapeMenu() {
    printf("\nChoose shape type:\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("Enter shape type: ");
}

/* ── main ───────────────────────────────────────────────────────────── */

int main() {
    clearPicture();

    int choice;
    while (1) {
        printMainMenu();
        scanf("%d", &choice);

        if (choice == 1) {
            /* Add object */
            printShapeMenu();
            int shapeType;
            scanf("%d", &shapeType);

            Object o;
            o.type = (ShapeType)shapeType;

            if (shapeType == SHAPE_LINE) {
                printf("Enter x1 y1 x2 y2: ");
                scanf("%d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3]);
            } else if (shapeType == SHAPE_RECT) {
                printf("Enter top-left x y and bottom-right x y: ");
                scanf("%d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3]);
            } else if (shapeType == SHAPE_CIRCLE) {
                printf("Enter center x y and radius: ");
                scanf("%d %d %d",
                      &o.params[0], &o.params[1], &o.params[2]);
            } else if (shapeType == SHAPE_TRIANGLE) {
                printf("Enter x1 y1 x2 y2 x3 y3: ");
                scanf("%d %d %d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3],
                      &o.params[4], &o.params[5]);
            } else {
                printf("Invalid shape type.\n");
                continue;
            }

            if (objectCount < MAX_OBJECTS) {
                objects[objectCount] = o;
                printf("Object added with index %d.\n", objectCount);
                objectCount++;
                redrawAll();
            } else {
                printf("Object store full.\n");
            }

        } else if (choice == 2) {
            /* Delete object */
            printf("Enter index to delete: ");
            int idx;
            scanf("%d", &idx);
            if (idx < 0 || idx >= objectCount) {
                printf("Invalid index.\n");
            } else {
                for (int i = idx; i < objectCount - 1; i++)
                    objects[i] = objects[i + 1];
                objectCount--;
                printf("Object %d deleted.\n", idx);
                redrawAll();
            }

        } else if (choice == 3) {
            /* Modify object */
            printf("Enter index to modify: ");
            int idx;
            scanf("%d", &idx);
            if (idx < 0 || idx >= objectCount) {
                printf("Invalid index.\n");
                continue;
            }
            printShapeMenu();
            int shapeType;
            scanf("%d", &shapeType);

            Object o;
            o.type = (ShapeType)shapeType;

            if (shapeType == SHAPE_LINE) {
                printf("Enter x1 y1 x2 y2: ");
                scanf("%d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3]);
            } else if (shapeType == SHAPE_RECT) {
                printf("Enter top-left x y and bottom-right x y: ");
                scanf("%d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3]);
            } else if (shapeType == SHAPE_CIRCLE) {
                printf("Enter center x y and radius: ");
                scanf("%d %d %d",
                      &o.params[0], &o.params[1], &o.params[2]);
            } else if (shapeType == SHAPE_TRIANGLE) {
                printf("Enter x1 y1 x2 y2 x3 y3: ");
                scanf("%d %d %d %d %d %d",
                      &o.params[0], &o.params[1],
                      &o.params[2], &o.params[3],
                      &o.params[4], &o.params[5]);
            } else {
                printf("Invalid shape type.\n");
                continue;
            }
            objects[idx] = o;
            printf("Object %d modified.\n", idx);
            redrawAll();

        } else if (choice == 4) {
            /* Display picture */
            printf("\n");
            displayPicture();

        } else if (choice == 5) {
            /* List objects */
            if (objectCount == 0) {
                printf("No objects.\n");
            } else {
                for (int i = 0; i < objectCount; i++) {
                    Object *o = &objects[i];
                    printf("[%d] ", i);
                    switch (o->type) {
                        case SHAPE_LINE:
                            printf("Line (%d,%d)->(%d,%d)\n",
                                   o->params[0], o->params[1],
                                   o->params[2], o->params[3]);
                            break;
                        case SHAPE_RECT:
                            printf("Rectangle (%d,%d)->(%d,%d)\n",
                                   o->params[0], o->params[1],
                                   o->params[2], o->params[3]);
                            break;
                        case SHAPE_CIRCLE:
                            printf("Circle center(%d,%d) r=%d\n",
                                   o->params[0], o->params[1], o->params[2]);
                            break;
                        case SHAPE_TRIANGLE:
                            printf("Triangle (%d,%d),(%d,%d),(%d,%d)\n",
                                   o->params[0], o->params[1],
                                   o->params[2], o->params[3],
                                   o->params[4], o->params[5]);
                            break;
                    }
                }
            }

        } else if (choice == 0) {
            printf("Goodbye.\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
