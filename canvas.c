/*
 * canvas.c  –  2D character canvas implementation
 */
#include "canvas.h"
#include "console.h"   /* con_set_color, COL_* */
#include <stdio.h>
#include <string.h>

/* ── Global canvas buffer ────────────────────────────────────────── */
char canvas[CANVAS_H][CANVAS_W];

/* ── canvas_clear ────────────────────────────────────────────────── */
void canvas_clear(void) {
    memset(canvas, EMPTY, sizeof(canvas));
}

/* ── canvas_put ──────────────────────────────────────────────────── */
void canvas_put(int x, int y, char ch) {
    if (x >= 0 && x < CANVAS_W && y >= 0 && y < CANVAS_H)
        canvas[y][x] = ch;
}

/* ── display_canvas ──────────────────────────────────────────────── */
void display_canvas(void) {
    /* ── top border ── */
    con_set_color(COL_BORDER);
    putchar('+');
    for (int c = 0; c < CANVAS_W; c++) putchar('-');
    puts("+");

    for (int r = 0; r < CANVAS_H; r++) {
        con_set_color(COL_BORDER);
        putchar('|');

        for (int c = 0; c < CANVAS_W; c++) {
            char ch = canvas[r][c];
            if      (ch == '*') con_set_color(COL_STAR);
            else if (ch == '_') con_set_color(COL_UNDER);
            else                con_set_color(COL_EMPTY);
            putchar(ch);
        }

        con_set_color(COL_BORDER);
        puts("|");
    }

    /* ── bottom border ── */
    putchar('+');
    for (int c = 0; c < CANVAS_W; c++) putchar('-');
    puts("+");

    con_set_color(COL_NORMAL);
}
