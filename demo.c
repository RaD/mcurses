/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file demo.c - demo program using mcurses lib
 *
 * Copyright (c) 2011-2014 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>

#if defined (unix)
#include <unistd.h>
#elif defined (__SDCC_z80)
  // notthing to do
#else // AVR
#include <avr/io.h>
#include <util/delay.h>
#endif

#include "mcurses.h"

#if defined (unix)
#define         PAUSE(x)                        { refresh(); if (!fast) usleep (1000 * x); }
static char *   myitoa(uint8_t x, char * buf)   { sprintf (buf, "%d", x); return buf; }
#define         PSTR(x)                         (x)
#elif defined (__SDCC_z80)
#warning        SDCC Z80 port of this demo program is not fully functional yet
#define         PAUSE(x)                        { refresh(); /* TODO */ }
static char *   myitoa(uint8_t x, char * buf)   { sprintf (buf, "%d", x); return buf; }
#define         PSTR(x)                         (x)
#else // AVR
#define         PAUSE(x)                        { refresh(); if (!fast) _delay_ms (x); }
#define         myitoa(x,buf)                   itoa ((x), buf, 10)
#endif

static uint8_t  fast;

static uint8_t  hanoi_pole_height[3];
static uint8_t  hanoi_number_of_rings;

// set_position: set position on pole
static void
hanoi_set_position (uint8_t pole, uint8_t ring)
{
    uint8_t column;
    uint8_t line;

    column = pole * 20 - ring - 3;
    line = hanoi_number_of_rings + 5 - hanoi_pole_height[pole - 1];
    move (line, column);
}

// erase ring on pole
static void
hanoi_erase_ring (uint8_t ring, uint8_t pole)
{
    uint8_t i;
    hanoi_set_position (pole, ring + 1);
    hanoi_pole_height[pole - 1]--;

    i = ring + 3;

    while (i--)
    {
        addch (' ');
    }

    if (hanoi_pole_height[pole - 1] - 1 == hanoi_number_of_rings)
    {
        addch (' ');
    }
    else
    {
        addch ('|');
    }

    i = ring + 3;

    while (i--)
    {
        addch (' ');
    }

}

// draw ring on pole
static void
hanoi_draw_ring (uint8_t ring, uint8_t pole)
{
    uint8_t i;

    hanoi_pole_height[pole - 1]++;
    hanoi_set_position (pole, ring);

    i = 2 * ring + 5;

    while (i--)
    {
        addch ('-');
    }
}


// move ring from pole to pole
static void
hanoi_move_ring (uint8_t ring, uint8_t from_pole, uint8_t to_pole)
{
    uint8_t height[3];
    uint8_t h;
    uint8_t i;

    height[0] = hanoi_pole_height[0];
    height[1] = hanoi_pole_height[1];
    height[2] = hanoi_pole_height[2];

    h = hanoi_number_of_rings - height[from_pole - 1] + 2;

    while (h)
    {
        hanoi_erase_ring (ring, from_pole);
        move (3, 0);
        PAUSE (100);
        hanoi_pole_height[from_pole-1]++;
        hanoi_draw_ring (ring, from_pole);
        move (3, 0);
        PAUSE (100);
        h--;
    }

    if (from_pole > to_pole)
    {
        for (i = 0; i < 20 * (from_pole - to_pole); i++)
        {
            mvdelch (3,0);
            PAUSE (25);
        }
    }
    else
    {
        for (i = 0; i < 20 * (to_pole - from_pole); i++)
        {
            mvinsch (3, 0, ' ');
            PAUSE (25);
        }
    }

    i = hanoi_number_of_rings - height[to_pole - 1] + 1;
    hanoi_pole_height[to_pole - 1] = hanoi_number_of_rings + 1;

    while (i)
    {
        hanoi_draw_ring (ring, to_pole);
        move (3, 0);
        PAUSE (100);
        hanoi_erase_ring (ring, to_pole);
        move (3, 0);
        PAUSE (100);
        hanoi_pole_height[to_pole - 1]--;
        i--;
    }

    hanoi_draw_ring (ring, to_pole);
    move (3, 0);
    PAUSE (100);

    hanoi_pole_height[from_pole - 1] = height[from_pole - 1] - 1;
    hanoi_pole_height[to_pole   - 1] = height[to_pole   - 1] + 1;
}

static void
hanoi (uint8_t nrings, uint8_t n1, uint8_t n2, uint8_t n3)
{
    uint8_t n;

    if (nrings == 0)
    {
        return;
    }

    n = nrings - 1;
    hanoi (n, n1, n3, n2);
    hanoi_move_ring (n, n1, n2);
    hanoi (n, n3, n2, n1);
}

static void
hanoi_draw_poles (void)
{
    uint8_t ring;
    uint8_t height_poles;
    uint8_t h;
    uint8_t i;
    uint8_t j;

    ring = hanoi_number_of_rings;
    height_poles = 1 + hanoi_number_of_rings;
    h = height_poles;

    while (h--)
    {
        for (i = 0; i < 3; i++)
        {
            mvaddch (h + 4, i * 20 + 19, '|');
        }
    }

    move (height_poles + 4, 0);

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 19; j++)
        {
            addch ('-');
        }

        if (i != 3)
        {
            addch ('^');
        }
    }

    while (ring)
    {
        hanoi_draw_ring (ring, 1);
        ring--;
    }
}

static void
show_top_line_P (const char * str)
{
    int     col;

    move (1, 0);
    attrset (A_BOLD | F_WHITE | B_BLUE);

    for (col = 0; col < COLS; col++)
    {
        addch (' ');
    }

    mvaddstr_P (1, 2, str);
    attrset (A_NORMAL);
}

static void
show_bottom_line_P (const char * str)
{
    uint8_t col;

    move (LINES - 3, 0);
    attrset (A_BOLD | F_WHITE | B_BLUE);

    for (col = 0; col < COLS; col++)
    {
        addch (' ');
    }

    mvaddstr_P (LINES - 3, 2, str);
    attrset (A_NORMAL);
}

static void
message_P (const char * msg)
{
    move (LINES - 2, 0);
    addstr_P (msg);
    clrtoeol ();
}

static void
shift_left (uint8_t y, uint8_t x, uint8_t ch)
{
    uint8_t col;

    move (y, COLS - 2);
    addch (ch);
    move (y, x);

    for (col = COLS - 2; col > x; col--)
    {
        PAUSE (5);
        delch ();
    }
}

static void
shift_left_str (uint8_t y, uint8_t x, char * str)
{
    char *  s;
    uint8_t xx = x;

    attrset (F_RED);

    for (s = str; *s; s++)
    {
        if (*s != ' ')
        {
            shift_left (y, xx, *s);
        }
        xx++;
    }

    move (y, x);
    attrset (A_REVERSE);

    for (s = str; *s; s++)
    {
        addch (*s);
        PAUSE (25);
    }

    move (y, x);
    attrset (F_BLUE);

    for (s = str; *s; s++)
    {
        addch (*s);
        PAUSE (25);
    }
}

static void
screen_demo (void)
{
    char    buf[10];
    uint8_t line;
    uint8_t col;

    clear ();

    show_top_line_P (PSTR("TOP LINE 2"));
    show_bottom_line_P (PSTR("BOTTOM LINE 22"));
    setscrreg (2, LINES - 4);

    if (fast)
    {
        mvaddstr_P (10, 20, PSTR("MCURSES LIB DEMO IN FAST MOTION"));
    }
    else
    {
        shift_left_str (10, 20, "MCURSES LIB DEMO IN SLOW MOTION");
    }

    for (line = 0; line < 5; line++)
    {
        scroll ();
        PAUSE (200);
    }

    move (5, 15);
    for (line = 0; line < 5; line++)
    {
        insertln ();
        PAUSE (200);
    }

    move (10, 18);
    for (col = 0; col < 5; col ++)
    {
        insch (' ');
        PAUSE (200);
    }

    move (10, 18);
    for (col = 0; col < 5; col ++)
    {
        delch ();
        PAUSE (200);
    }

    clear ();

    show_top_line_P (PSTR("TOP LINE 2"));
    show_bottom_line_P (PSTR("BOTTOM LINE 22"));

    message_P (PSTR("line positioning test"));

    for (line = 2; line <= LINES - 4; line++)
    {
        move (line, 0);
        addstr (myitoa (line + 1, buf));
    }

    PAUSE (700);

    message_P (PSTR("BOLD attribute test"));
    attrset (A_BOLD);
    mvaddstr_P (10, 10, PSTR("BOLD"));
    attrset (A_NORMAL);
    PAUSE (700);

    message_P (PSTR("REVERSE attribute test"));
    attrset (A_REVERSE);
    mvaddstr_P (11, 10, PSTR("REVERSE"));
    attrset (A_NORMAL);
    PAUSE (700);

    message_P (PSTR("insert character test"));
    for (col = 10; col <= 22; col += 2)
    {
        mvinsch (11, col, ' ');
    }
    move (11, col + 1);
    PAUSE (700);

    message_P (PSTR("UNDERLINE attribute test"));
    attrset (A_UNDERLINE);
    mvaddstr_P (12, 10, PSTR("UNDERLINE"));
    attrset (A_NORMAL);
    PAUSE (1000);

    message_P (PSTR("insert line test"));
    move (11, 10);
    insertln ();
    PAUSE (1000);

    addstr_P (PSTR("Inserted line, will be deleted soon..."));
    PAUSE (1000);

    message_P (PSTR("delete character test"));
    for (col = 10; col <= 16; col += 1)
    {
        mvdelch (12, col);
    }
    move (12, 18);
    PAUSE (1000);

    message_P (PSTR("delete line test"));
    move (11, 10);
    deleteln ();
    PAUSE (1000);

    message_P (PSTR("scroll up line test"));
    for (line = 0; line < LINES - 4; line++)
    {
        scroll ();
        PAUSE (50);
    }

    move ( 8, 20); attrset (A_BOLD | F_BLACK   | B_WHITE); addstr ("BLACK");
    move ( 9, 20); attrset (A_BOLD | F_RED     | B_WHITE); addstr ("RED");
    move (10, 20); attrset (A_BOLD | F_GREEN   | B_WHITE); addstr ("GREEN");
    move (11, 20); attrset (A_BOLD | F_YELLOW  | B_WHITE); addstr ("YELLOW");
    move (12, 20); attrset (A_BOLD | F_BLUE    | B_WHITE); addstr ("BLUE");
    move (13, 20); attrset (A_BOLD | F_MAGENTA | B_WHITE); addstr ("MAGENTA");
    move (14, 20); attrset (A_BOLD | F_CYAN    | B_WHITE); addstr ("CYAN");
    move (15, 20); attrset (A_BOLD | F_WHITE   | B_BLACK); addstr ("WHITE");
    move (16, 20); attrset (A_NORMAL); addstr ("normal");

    move ( 8, 50); attrset (A_BOLD | B_BLACK   | F_WHITE); addstr ("BLACK");
    move ( 9, 50); attrset (A_BOLD | B_RED     | F_WHITE); addstr ("RED");
    move (10, 50); attrset (A_BOLD | B_GREEN   | F_WHITE); addstr ("GREEN");
    move (11, 50); attrset (A_BOLD | B_YELLOW  | F_BLACK); addstr ("YELLOW");
    move (12, 50); attrset (A_BOLD | B_BLUE    | F_WHITE); addstr ("BLUE");
    move (13, 50); attrset (A_BOLD | B_MAGENTA | F_WHITE); addstr ("MAGENTA");
    move (14, 50); attrset (A_BOLD | B_CYAN    | F_WHITE); addstr ("CYAN");
    move (15, 50); attrset (A_BOLD | B_WHITE   | F_BLACK); addstr ("WHITE");
    move (16, 50); attrset (A_NORMAL); addstr ("normal");
    PAUSE (2000);
}

static void
temperature ()
{
    uint8_t idx;
    uint8_t t;
    uint8_t x;
    uint8_t loop;
    char    buf[10];
    unsigned char temp[15] = { 0, 8, 15, 21, 26, 30, 32, 35, 32, 30, 26, 21, 15, 8, 0 };

    curs_set (0);                                                       // set cursor invisible
    clear ();
    show_top_line_P (PSTR("Temperatures in a disk storage"));
    show_bottom_line_P (PSTR(""));

    for (loop = 0; loop < 30; loop++)
    {
        for (idx = 0; idx < 15; idx++)
        {
            if (temp[idx] > 30)
            {
                attrset (B_RED);
            }
            else if (temp[idx] > 20)
            {
                attrset (B_BROWN);
            }
            else
            {
                attrset (B_GREEN);
            }

            mvaddstr (idx + 4, 5, "Disk ");
            myitoa (idx + 1, buf);

            if (idx + 1 < 10)
            {
                addch (' ');
            }

            addstr (buf);
            addstr (": ");
            myitoa (temp[idx] + 20, buf);
            addstr (buf);
            addch ('°');
            attrset (A_NORMAL);

            move (idx + 4, 20);

            addch (ACS_LTEE);

            for (t = 0; t < temp[idx]; t++)
            {
                addch (ACS_HLINE);
            }

            clrtoeol ();

            x = rand() & 0x1F;

            if (x == 0x01)
            {
                if (temp[idx] < 55)
                {
                    temp[idx]++;
                }
            }
            else if (x == 0x02)
            {
                if (temp[idx] > 0)
                {
                    temp[idx]--;
                }
            }
        }
        PAUSE (100);
    }

    curs_set (1);                                                       // set cursor visible (normal)
}

static void
drawbox (uint8_t y, uint8_t x, uint8_t h, uint8_t w)
{
    uint8_t line;
    uint8_t col;

    move (y, x);
    addch (ACS_ULCORNER);
    for (col = 0; col < w - 2; col++)
    {
        addch (ACS_HLINE);
    }
    addch (ACS_URCORNER);

    for (line = 0; line < h - 2; line++)
    {
        move (line + y + 1, x);
        addch (ACS_VLINE);
        move (line + y + 1, x + w - 1);
        addch (ACS_VLINE);
    }

    move (y + h - 1, x);
    addch (ACS_LLCORNER);
    for (col = 0; col < w - 2; col++)
    {
        addch (ACS_HLINE);
    }
    addch (ACS_LRCORNER);
}

int
main ()
{
    char    buf[10];
    uint8_t idx;
    uint8_t cnt;
    uint8_t ch;

    initscr ();

    while (1)
    {
        screen_demo ();
        clear ();
        mvaddstr_P (10, 10, PSTR("Now the same in full speed... "));

        for (idx = 3; idx > 0; idx--)
        {
            move (10, 40);
            myitoa(idx, buf);
            addstr (buf);
            PAUSE (1000);
        }

        fast = 1;
        screen_demo ();
        clear ();
        fast = 0;

        mvaddstr_P (10, 10, PSTR("Whoops, too fast? Let's do something else... "));
        PAUSE (2000);

        clear ();
        hanoi_number_of_rings = 4;
        hanoi_pole_height[0] = 0;
        hanoi_pole_height[1] = 0;
        hanoi_pole_height[2] = 0;

        hanoi_draw_poles ();
        hanoi (hanoi_number_of_rings, 1, 2, 3);
        PAUSE (1000);

        temperature ();

        clear ();
        drawbox (6, 20, 10, 20);

        for (idx = 1; idx <= 6; idx++)
        {
            mvaddstr_P (idx + 7, 23, PSTR("This is line "));
            addstr (myitoa(idx, buf));
            PAUSE (400);
        }
        PAUSE (1000);
        clear ();

        nodelay (TRUE);             // set nodelay, getch() will then return immediately
        cnt = 0;

        while (1)
        {
            move (11, 10);
            addstr_P (PSTR("Timeout Counter: "));
            addstr (myitoa(cnt, buf));
            addstr_P (PSTR(" tenths of a sec"));
            clrtoeol ();

            move (10, 10);
            addstr_P (PSTR("Press a key (2x ESC or 5 seconds timeout exits): "));

            ch = getch ();

            switch (ch)
            {
                case '\t':          addstr_P (PSTR("TAB"));         break;
                case '\r':          addstr_P (PSTR("CR"));          break;
                case KEY_ESCAPE:    addstr_P (PSTR("KEY_ESCAPE"));  break;
                case KEY_DOWN:      addstr_P (PSTR("KEY_DOWN"));    break;
                case KEY_UP:        addstr_P (PSTR("KEY_UP"));      break;
                case KEY_LEFT:      addstr_P (PSTR("KEY_LEFT"));    break;
                case KEY_RIGHT:     addstr_P (PSTR("KEY_RIGHT"));   break;
                case KEY_HOME:      addstr_P (PSTR("KEY_HOME"));    break;
                case KEY_DC:        addstr_P (PSTR("KEY_DC"));      break;
                case KEY_IC:        addstr_P (PSTR("KEY_IC"));      break;
                case KEY_NPAGE:     addstr_P (PSTR("KEY_NPAGE"));   break;
                case KEY_PPAGE:     addstr_P (PSTR("KEY_PPAGE"));   break;
                case KEY_END:       addstr_P (PSTR("KEY_END"));     break;
                case KEY_BTAB:      addstr_P (PSTR("KEY_BTAB"));    break;
                case KEY_F(1):      addstr_P (PSTR("KEY_F(1)"));    break;
                case KEY_F(2):      addstr_P (PSTR("KEY_F(2)"));    break;
                case KEY_F(3):      addstr_P (PSTR("KEY_F(3)"));    break;
                case KEY_F(4):      addstr_P (PSTR("KEY_F(4)"));    break;
                case KEY_F(5):      addstr_P (PSTR("KEY_F(5)"));    break;
                case KEY_F(6):      addstr_P (PSTR("KEY_F(6)"));    break;
                case KEY_F(7):      addstr_P (PSTR("KEY_F(7)"));    break;
                case KEY_F(8):      addstr_P (PSTR("KEY_F(8)"));    break;
                case KEY_F(9):      addstr_P (PSTR("KEY_F(9)"));    break;
                case KEY_F(10):     addstr_P (PSTR("KEY_F(10)"));   break;
                case KEY_F(11):     addstr_P (PSTR("KEY_F(11)"));   break;
                case KEY_F(12):     addstr_P (PSTR("KEY_F(12)"));   break;
                case ERR:           PAUSE(100); cnt++;              break;
                default:            addch (ch);                     break;
            }

            if (ch != ERR)
            {
                cnt = 0;
                clrtoeol ();

                if (ch == KEY_ESCAPE)
                {
                    PAUSE(500);
                    break;
                }
            }
            else if (cnt >= 50 + 1)
            {
                break;
            }
        }

#if defined (unix) || defined (__SDCC_z80)
        break;
#endif
    }
    endwin ();

    return 0;
}

