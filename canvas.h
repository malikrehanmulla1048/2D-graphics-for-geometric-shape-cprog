/*
 * canvas.h  –  2D character canvas interface
 *
 * The canvas is a 2D array of characters (CANVAS_H rows × CANVAS_W cols).
 * All drawing primitives write into this array; display_canvas() renders it.
 */
#ifndef CANVAS_H
#define CANVAS_H

#define CANVAS_W   78   /* usable width  (fits 80-col terminal) */
#define CANVAS_H   36   /* usable height */
#define EMPTY      ' '

/* The shared canvas buffer */
extern char canvas[CANVAS_H][CANVAS_W];

/* Initialise every cell to EMPTY */
void canvas_clear(void);

/*
 * Write character ch at column x, row y.
 * Out-of-bounds writes are silently ignored.
 */
void canvas_put(int x, int y, char ch);

/* Render the canvas to stdout with a decorative border */
void display_canvas(void);

#endif /* CANVAS_H */
