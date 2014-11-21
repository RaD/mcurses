/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file mcurses.c - mcurses lib
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
#include <string.h>

#ifdef unix
#include <termio.h>
#include <fcntl.h>
#define PROGMEM
#define PSTR(x)                                 (x)
#define pgm_read_byte(s)                        (*s)
#elif (defined __SDCC_z80)
#define PROGMEM
#define PSTR(x)                                 (x)
#define pgm_read_byte(s)                        (*s)
#else // AVR
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#endif

#include "mcurses.h"

#define SEQ_CSI                                 PSTR("\033[")                   // code introducer
#define SEQ_CLEAR                               PSTR("\033[2J")                 // clear screen
#define SEQ_CLRTOBOT                            PSTR("\033[J")                  // clear to bottom
#define SEQ_CLRTOEOL                            PSTR("\033[K")                  // clear to end of line
#define SEQ_DELCH                               PSTR("\033[P")                  // delete character
#define SEQ_NEXTLINE                            PSTR("\033E")                   // goto next line (scroll up at end of scrolling region)
#define SEQ_INSERTLINE                          PSTR("\033[L")                  // insert line
#define SEQ_DELETELINE                          PSTR("\033[M")                  // delete line
#define SEQ_ATTRSET                             PSTR("\033[0")                  // set attributes, e.g. "\033[0;7;1m"
#define SEQ_ATTRSET_REVERSE                     PSTR(";7")                      // reverse
#define SEQ_ATTRSET_UNDERLINE                   PSTR(";4")                      // underline
#define SEQ_ATTRSET_BLINK                       PSTR(";5")                      // blink
#define SEQ_ATTRSET_BOLD                        PSTR(";1")                      // bold
#define SEQ_ATTRSET_DIM                         PSTR(";2")                      // dim
#define SEQ_ATTRSET_FCOLOR                      PSTR(";3")                      // forground color
#define SEQ_ATTRSET_BCOLOR                      PSTR(";4")                      // background color
#define SEQ_INSERT_MODE                         PSTR("\033[4h")                 // set insert mode
#define SEQ_REPLACE_MODE                        PSTR("\033[4l")                 // set replace mode
#define SEQ_RESET_SCRREG                        PSTR("\033[r")                  // reset scrolling region
#define SEQ_LOAD_G1                             PSTR("\033)0")                  // load G1 character set
#define SEQ_CURSOR_VIS                          PSTR("\033[?25")                // set cursor visible/not visible

static uint8_t                                  mcurses_scrl_start = 0;         // start of scrolling region, default is 0
static uint8_t                                  mcurses_scrl_end = LINES - 1;   // end of scrolling region, default is last line
static uint8_t                                  mcurses_nodelay;                // nodelay flag
uint8_t                                         mcurses_cury;                   // current y position of cursor, public (getyx())
uint8_t                                         mcurses_curx;                   // current x position of cursor, public (getyx())

#if defined(unix)

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init, done, putc, getc, nodelay, flush for UNIX or LINUX
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static struct termio                            mcurses_oldmode;
static struct termio                            mcurses_newmode;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_init (void)
{
    int     fd;

    fd = fileno (stdin);

    (void) ioctl (fd, TCGETA, &mcurses_oldmode);
    (void) ioctl (fd, TCGETA, &mcurses_newmode);

    mcurses_newmode.c_lflag &= ~ICANON;                                         // switch off canonical input
    mcurses_newmode.c_lflag &= ~ECHO;                                           // switch off echo
    mcurses_newmode.c_iflag &= ~ICRNL;                                          // switch off CR->NL mapping
    mcurses_newmode.c_oflag &= ~TAB3;                                           // switch off TAB conversion
    mcurses_newmode.c_cc[VINTR] = '\377';                                       // disable VINTR VQUIT
    mcurses_newmode.c_cc[VQUIT] = '\377';                                       // but don't touch VSWTCH
    mcurses_newmode.c_cc[VMIN] = 1;                                             // block input:
    mcurses_newmode.c_cc[VTIME] = 0;                                            // one character
    (void) ioctl (fd, TCSETAW, &mcurses_newmode);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_done (void)
{
    int     fd;

    fd = fileno (stdin);

    (void) ioctl (fd, TCSETAW, &mcurses_oldmode);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_putc (uint8_t ch)
{
    putchar (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_getc (void)
{
    uint8_t ch;

    ch = getchar ();

    return (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_nodelay (uint8_t flag)
{
    int     fd;
    int     fl;

    fd = fileno (stdin);

    if ((fl = fcntl (fd, F_GETFL, 0)) >= 0)
    {
        if (flag)
        {
            fl |= O_NDELAY;
        }
        else
        {
            fl &= ~O_NDELAY;
        }
        (void) fcntl (fd, F_SETFL, fl);
        mcurses_nodelay = flag;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_flush_output ()
{
    fflush (stdout);
}

#elif defined (__SDCC_z80)
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_init (void)
{
  // Deine UART Initialisierung, falls notwendig.
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_done (void)
{
  // Kann leer bleiben.
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_putc (uint8_t ch)
{
    putchar (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_getc (void)
{
    uint8_t ch;

    ch = getchar ();

    return (ch);
}

static uint8_t z80_nodelay;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_nodelay (uint8_t flag)
{
    z80_nodelay = flag; // TODO
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_flush_output ()
{
  // leer lassen
}

#else // AVR

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init, done, putc, getc for AVR
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define BAUD                                    MCURSES_BAUD
#include <util/setbaud.h>

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Newer ATmegas, e.g. ATmega88, ATmega168
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR0H

#define UART0_UBRRH                             UBRR0H
#define UART0_UBRRL                             UBRR0L
#define UART0_UCSRA                             UCSR0A
#define UART0_UCSRB                             UCSR0B
#define UART0_UCSRC                             UCSR0C
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE0)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ01)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ00)
#ifdef URSEL0
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL0)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN0)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN0)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE0)
#define UART0_UDR                               UDR0
#define UART0_U2X                               U2X0
#define UART0_RXC                               RXC0

#ifdef USART0_TXC_vect                                                  // e.g. ATmega162 with 2 UARTs
#define UART0_TXC_vect                          USART0_TXC_vect
#define UART0_RXC_vect                          USART0_RXC_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#elif defined(USART0_TX_vect)                                           // e.g. ATmega644 with 2 UARTs
#define UART0_TXC_vect                          USART0_TX_vect
#define UART0_RXC_vect                          USART0_RX_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#else                                                                   // e.g. ATmega168 with 1 UART
#define UART0_TXC_vect                          USART_TX_vect
#define UART0_RXC_vect                          USART_RX_vect
#define UART0_UDRE_vect                         USART_UDRE_vect
#endif

#define UART0_UDRIE                             UDRIE0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * ATmegas with 2nd UART, e.g. ATmega162
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR1H
#define UART1_UBRRH                             UBRR1H
#define UART1_UBRRL                             UBRR1L
#define UART1_UCSRA                             UCSR1A
#define UART1_UCSRB                             UCSR1B
#define UART1_UCSRC                             UCSR1C
#define UART1_UDRE_BIT_VALUE                    (1<<UDRE1)
#define UART1_UCSZ1_BIT_VALUE                   (1<<UCSZ11)
#define UART1_UCSZ0_BIT_VALUE                   (1<<UCSZ10)
#ifdef URSEL1
#define UART1_URSEL_BIT_VALUE                   (1<<URSEL1)
#else
#define UART1_URSEL_BIT_VALUE                   (0)
#endif
#define UART1_TXEN_BIT_VALUE                    (1<<TXEN1)
#define UART1_RXEN_BIT_VALUE                    (1<<RXEN1)
#define UART1_RXCIE_BIT_VALUE                   (1<<RXCIE1)
#define UART1_UDR                               UDR1
#define UART1_U2X                               U2X1
#define UART1_RXC                               RXC1

#ifdef USART1_TXC_vect                                                  // e.g. ATmega162 with 2 UARTs
#define UART1_TXC_vect                          USART1_TXC_vect
#define UART1_RXC_vect                          USART1_RXC_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#else                                                                   // e.g. ATmega644 with 2 UARTs
#define UART1_TXC_vect                          USART1_TX_vect
#define UART1_RXC_vect                          USART1_RX_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#endif

#define UART1_UDRIE                             UDRIE1
#endif // UBRR1H

#else

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * older ATmegas, e.g. ATmega8, ATmega16
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define UART0_UBRRH                             UBRRH
#define UART0_UBRRL                             UBRRL
#define UART0_UCSRA                             UCSRA
#define UART0_UCSRB                             UCSRB
#define UART0_UCSRC                             UCSRC
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ1)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ0)
#ifdef URSEL
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE)
#define UART0_UDR                               UDR
#define UART0_U2X                               U2X
#define UART0_RXC                               RXC
#define UART0_UDRE_vect                         USART_UDRE_vect
#define UART0_TXC_vect                          USART_TXC_vect
#define UART0_RXC_vect                          USART_RXC_vect
#define UART0_UDRIE                             UDRIE

#endif

#define UART_TXBUFLEN                           64                              // 64 Bytes ringbuffer for UART
#define UART_RXBUFLEN                           16                              // 16 Bytes ringbuffer for UART

static volatile uint8_t uart_txbuf[UART_TXBUFLEN];                              // tx ringbuffer
static volatile uint8_t uart_txsize = 0;                                        // tx size
static volatile uint8_t uart_rxbuf[UART_RXBUFLEN];                              // rx ringbuffer
static volatile uint8_t uart_rxsize = 0;                                        // rx size

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_init (void)
{
    UART0_UBRRH = UBRRH_VALUE;                                                  // set baud rate
    UART0_UBRRL = UBRRL_VALUE;

#if USE_2X
    UART0_UCSRA |= (1<<UART0_U2X);
#else
    UART0_UCSRA &= ~(1<<UART0_U2X);
#endif

    UART0_UCSRC = UART0_UCSZ1_BIT_VALUE | UART0_UCSZ0_BIT_VALUE | UART0_URSEL_BIT_VALUE;    // 8 bit, no parity
    UART0_UCSRB |= UART0_TXEN_BIT_VALUE | UART0_RXEN_BIT_VALUE | UART0_RXCIE_BIT_VALUE;     // enable UART TX, RX, and RX interrupt
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_done (void)
{
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_putc (uint8_t ch)
{
    static uint8_t uart_txstop  = 0;                                            // tail

    while (uart_txsize >= UART_TXBUFLEN)                                        // buffer full?
    {                                                                           // yes
        ;                                                                       // wait
    }

    uart_txbuf[uart_txstop++] = ch;                                             // store character

    if (uart_txstop >= UART_TXBUFLEN)                                           // at end of ringbuffer?
    {                                                                           // yes
        uart_txstop = 0;                                                        // reset to beginning
    }

    cli();
    uart_txsize++;                                                              // increment used size
    sei();

    UART0_UCSRB |= (1 << UART0_UDRIE);                                          // enable UDRE interrupt

#if 0
    while (!(UART0_UCSRA & UART0_UDRE_BIT_VALUE))
    {
        ;
    }

    UART0_UDR = ch;
#endif
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_getc (void)
{
    static uint8_t  uart_rxstart = 0;                                           // head
    uint8_t         ch;

    while (uart_rxsize == 0)                                                    // rx buffer empty?
    {                                                                           // yes, wait
        if (mcurses_nodelay)
        {                                                                       // or if nodelay set, return ERR
            return (ERR);
        }
    }

    ch = uart_rxbuf[uart_rxstart++];                                            // get character from ringbuffer

    if (uart_rxstart == UART_RXBUFLEN)                                          // at end of rx buffer?
    {                                                                           // yes
        uart_rxstart = 0;                                                       // reset to beginning
    }

    cli();
    uart_rxsize--;                                                              // decrement size
    sei();

#if 0
    while (!(UART0_UCSRA & (1<<UART0_RXC)))                                     // character available?
    {                                                                           // no
        ;                                                                       // wait
    }

    ch = UART0_UDR;                                                             // read character from UDRx
#endif
    return (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_nodelay (uint8_t flag)
{
    mcurses_nodelay = flag;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_flush_output ()
{
    while (uart_txsize > 0)                                                     // tx buffer empty?
    {
        ;                                                                       // no, wait
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: UART interrupt handler, called if UART has received a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
ISR(UART0_RXC_vect)
{
    static uint8_t  uart_rxstop  = 0;                                           // tail
    uint8_t         ch;

    ch = UART0_UDR;

    if (uart_rxsize < UART_RXBUFLEN)                                            // buffer full?
    {                                                                           // no
        uart_rxbuf[uart_rxstop++] = ch;                                         // store character

        if (uart_rxstop >= UART_RXBUFLEN)                                       // at end of ringbuffer?
        {                                                                       // yes
            uart_rxstop = 0;                                                    // reset to beginning
        }

        uart_rxsize++;                                                          // increment used size
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: UART interrupt handler, called if UART is ready to send a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
ISR(UART0_UDRE_vect)
{
    static uint8_t  uart_txstart = 0;                                           // head
    uint8_t         ch;

    if (uart_txsize > 0)                                                        // tx buffer empty?
    {                                                                           // no
        ch = uart_txbuf[uart_txstart++];                                        // get character to send, increment offset

        if (uart_txstart == UART_TXBUFLEN)                                      // at end of tx buffer?
        {                                                                       // yes
            uart_txstart = 0;                                                   // reset to beginning
        }

        uart_txsize--;                                                          // decrement size

        UART0_UDR = ch;                                                         // write character, don't wait
    }
    else
    {
        UART0_UCSRB &= ~(1 << UART0_UDRIE);                                     // tx buffer empty, disable UDRE interrupt
    }
}

#endif // !unix

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_putc (uint8_t ch)
{
    mcurses_phyio_putc (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puts_P (const char * str)
{
    uint8_t ch;

    while ((ch = pgm_read_byte(str)) != '\0')
    {
        mcurses_putc (ch);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a 3/2/1 digit integer number (raw)
 *
 * Here we don't want to use sprintf (too big on AVR/Z80) or itoa (not available on Z80)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puti (uint8_t i)
{
    if (i >= 100)
    {
        uint8_t ii = i / 100;
        mcurses_putc (ii + '0');
        i -= 100 * ii;
        ii = i / 10;
        mcurses_putc (ii + '0');
        i -= 10 * ii;
    }
    else if (i >= 10)
    {
        uint8_t ii = i / 10;
        mcurses_putc (ii + '0');
        i -= 10 * ii;
    }

    mcurses_putc (i + '0');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: addch or insch a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CHARSET_G0      0
#define CHARSET_G1      1

static void
mcurses_addch_or_insch (uint8_t ch, uint8_t insert)
{
    static uint8_t  charset = 0xff;
    static uint8_t  insert_mode = FALSE;

    if (ch >= 0x80 && ch <= 0x9F)
    {
        if (charset != CHARSET_G1)
        {
            mcurses_putc ('\016');                                              // switch to G1 set
            charset = CHARSET_G1;
        }
        ch -= 0x20;                                                             // subtract offset to G1 characters
    }
    else
    {
        if (charset != CHARSET_G0)
        {
            mcurses_putc ('\017');                                              // switch to G0 set
            charset = CHARSET_G0;
        }
    }

    if (insert)
    {
        if (! insert_mode)
        {
            mcurses_puts_P (SEQ_INSERT_MODE);
            insert_mode = TRUE;
        }
    }
    else
    {
        if (insert_mode)
        {
            mcurses_puts_P (SEQ_REPLACE_MODE);
            insert_mode = FALSE;
        }
    }

    mcurses_putc (ch);
    mcurses_curx++;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: set scrolling region (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mysetscrreg (uint8_t top, uint8_t bottom)
{
    if (top == bottom)
    {
        mcurses_puts_P (SEQ_RESET_SCRREG);                                      // reset scrolling region
    }
    else
    {
        mcurses_puts_P (SEQ_CSI);
        mcurses_puti (top + 1);
        mcurses_putc (';');
        mcurses_puti (bottom + 1);
        mcurses_putc ('r');
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * move cursor (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mymove (uint8_t y, uint8_t x)
{
    mcurses_puts_P (SEQ_CSI);
    mcurses_puti (y + 1);
    mcurses_putc (';');
    mcurses_puti (x + 1);
    mcurses_putc ('H');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
initscr (void)
{
    mcurses_phyio_init ();
    mcurses_puts_P (SEQ_LOAD_G1);                                               // load graphic charset into G1
    clear ();
    move (0, 0);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addch (uint8_t ch)
{
    mcurses_addch_or_insch (ch, FALSE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr (char * str)
{
    while (*str)
    {
        mcurses_addch_or_insch (*str++, FALSE);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr_P (const char * str)
{
    uint8_t ch;

    while ((ch = pgm_read_byte(str)) != '\0')
    {
        mcurses_addch_or_insch (ch, FALSE);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set attribute(s)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
attrset (uint16_t attr)
{
    uint8_t         idx;

    mcurses_puts_P (SEQ_ATTRSET);

    idx = (attr & F_COLOR) >> 8;

    if (idx >= 1 && idx <= 8)
    {
        mcurses_puts_P (SEQ_ATTRSET_FCOLOR);
        mcurses_putc (idx - 1 + '0');
    }

    idx = (attr & B_COLOR) >> 12;

    if (idx >= 1 && idx <= 8)
    {
        mcurses_puts_P (SEQ_ATTRSET_BCOLOR);
        mcurses_putc (idx - 1 + '0');
    }

    if (attr & A_REVERSE)
    {
        mcurses_puts_P (SEQ_ATTRSET_REVERSE);
    }
    if (attr & A_UNDERLINE)
    {
        mcurses_puts_P (SEQ_ATTRSET_UNDERLINE);
    }
    if (attr & A_BLINK)
    {
        mcurses_puts_P (SEQ_ATTRSET_BLINK);
    }
    if (attr & A_BOLD)
    {
        mcurses_puts_P (SEQ_ATTRSET_BOLD);
    }
    if (attr & A_DIM)
    {
        mcurses_puts_P (SEQ_ATTRSET_DIM);
    }
    mcurses_putc ('m');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
move (uint8_t y, uint8_t x)
{
    mcurses_cury = y;
    mcurses_curx = x;
    mymove (y, x);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
deleteln (void)
{
    mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
    mymove (mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P (SEQ_DELETELINE);                                            // delete line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    move (mcurses_cury, mcurses_curx);                                          // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insertln (void)
{
    mysetscrreg (mcurses_cury, mcurses_scrl_end);                               // set scrolling region
    mymove (mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P (SEQ_INSERTLINE);                                            // insert line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    mymove (mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: scroll
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
scroll (void)
{
    mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
    mymove (mcurses_scrl_end, 0);                                               // goto to last line of scrolling region
    mcurses_puts_P (SEQ_NEXTLINE);                                              // next line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    mymove (mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clear (void)
{
    mcurses_puts_P (SEQ_CLEAR);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to bottom of screen
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtobot (void)
{
    mcurses_puts_P (SEQ_CLRTOBOT);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to end of line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtoeol (void)
{
    mcurses_puts_P (SEQ_CLRTOEOL);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete character at cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
delch (void)
{
    mcurses_puts_P (SEQ_DELCH);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insch (uint8_t ch)
{
    mcurses_addch_or_insch (ch, TRUE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set scrolling region
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
setscrreg (uint8_t t, uint8_t b)
{
    mcurses_scrl_start = t;
    mcurses_scrl_end = b;
}

void
curs_set (uint8_t visibility)
{
    mcurses_puts_P (SEQ_CURSOR_VIS);

    if (visibility == 0)
    {
        mcurses_putc ('l');
    }
    else
    {
        mcurses_putc ('h');
    }
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: refresh: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
refresh (void)
{
    mcurses_phyio_flush_output ();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
nodelay (uint8_t flag)
{
    if (mcurses_nodelay != flag)
    {
        mcurses_phyio_nodelay (flag);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read key
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_KEYS                ((KEY_F1 + 12) - 0x80)

static const char * function_keys[MAX_KEYS] =
{
    "B",                        // KEY_DOWN                 0x80                // Down arrow key
    "A",                        // KEY_UP                   0x81                // Up arrow key
    "D",                        // KEY_LEFT                 0x82                // Left arrow key
    "C",                        // KEY_RIGHT                0x83                // Right arrow key
    "1~",                       // KEY_HOME                 0x84                // Home key
    "3~",                       // KEY_DC                   0x85                // Delete character key
    "2~",                       // KEY_IC                   0x86                // Ins char/toggle ins mode key
    "6~",                       // KEY_NPAGE                0x87                // Next-page key
    "5~",                       // KEY_PPAGE                0x88                // Previous-page key
    "4~",                       // KEY_END                  0x89                // End key
    "Z",                        // KEY_BTAB                 0x8A                // Back tab key
#if 0 // VT400:
    "11~",                      // KEY_F(1)                 0x8B                // Function key F1
    "12~",                      // KEY_F(2)                 0x8C                // Function key F2
    "13~",                      // KEY_F(3)                 0x8D                // Function key F3
    "14~",                      // KEY_F(4)                 0x8E                // Function key F4
    "15~",                      // KEY_F(5)                 0x8F                // Function key F5
#else // Linux console
    "[A",                       // KEY_F(1)                 0x8B                // Function key F1
    "[B",                       // KEY_F(2)                 0x8C                // Function key F2
    "[C",                       // KEY_F(3)                 0x8D                // Function key F3
    "[D",                       // KEY_F(4)                 0x8E                // Function key F4
    "[E",                       // KEY_F(5)                 0x8F                // Function key F5
#endif
    "17~",                      // KEY_F(6)                 0x90                // Function key F6
    "18~",                      // KEY_F(7)                 0x91                // Function key F7
    "19~",                      // KEY_F(8)                 0x92                // Function key F8
    "20~",                      // KEY_F(9)                 0x93                // Function key F9
    "21~",                      // KEY_F(10)                0x94                // Function key F10
    "23~",                      // KEY_F(11)                0x95                // Function key F11
    "24~"                       // KEY_F(12)                0x96                // Function key F12
};

uint8_t
getch (void)
{
    char    buf[4];
    uint8_t ch;
    uint8_t idx;

    refresh ();
    ch = mcurses_phyio_getc ();

    if (ch == 0x7F)                                                             // BACKSPACE on VT200 sends DEL char
    {
        ch = KEY_BACKSPACE;                                                     // map it to '\b'
    }
    else if (ch == '\033')                                                      // ESCAPE
    {
        while ((ch = mcurses_phyio_getc ()) == ERR)
        {
            ;
        }

        if (ch == '\033')                                                       // 2 x ESCAPE
        {
            return KEY_ESCAPE;
        }
        else if (ch == '[')
        {
            for (idx = 0; idx < 3; idx++)
            {
                while ((ch = mcurses_phyio_getc ()) == ERR)
                {
                    ;
                }

                buf[idx] = ch;

                if ((ch >= 'A' && ch <= 'Z') || ch == '~')
                {
                    idx++;
                    break;
                }
            }

            buf[idx] = '\0';

            for (idx = 0; idx < MAX_KEYS; idx++)
            {
                if (! strcmp (buf, function_keys[idx]))
                {
                    ch = idx + 0x80;
                    break;
                }
            }

            if (idx == MAX_KEYS)
            {
                ch = KEY_ESCAPE;
            }
        }
    }

    return ch;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read string (with mini editor built-in)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
getnstr (char * str, uint8_t maxlen)
{
    uint8_t ch;
    uint8_t curlen = 0;
    uint8_t curpos = 0;
    uint8_t starty;
    uint8_t startx;
    uint8_t i;

    maxlen--;                               // reserve one byte in order to store '\0' in last position
    getyx (starty, startx);                 // get current cursor position

    while ((ch = getch ()) != KEY_CR)
    {
        switch (ch)
        {
            case KEY_LEFT:
                if (curpos > 0)
                {
                    curpos--;
                }
                break;
            case KEY_RIGHT:
                if (curpos < curlen)
                {
                    curpos++;
                }
                break;
            case KEY_HOME:
                curpos = 0;
                break;
            case KEY_END:
                curpos = curlen;
                break;
            case KEY_BACKSPACE:
                if (curpos > 0)
                {
                    curpos--;
                    curlen--;
                    move (starty, startx + curpos);

                    for (i = curpos; i < curlen; i++)
                    {
                        str[i] = str[i + 1];
                    }
                    str[i] = '\0';
                    delch();
                }
                break;

            case KEY_DC:
                if (curlen > 0)
                {
                    curlen--;
                    for (i = curpos; i < curlen; i++)
                    {
                        str[i] = str[i + 1];
                    }
                    str[i] = '\0';
                    delch();
                }
                break;

            default:
                if (curlen < maxlen && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127)      // printable ascii 7bit or printable 8bit ISO8859
                {
                    for (i = curlen; i > curpos; i--)
                    {
                        str[i] = str[i - 1];
                    }
                    insch (ch);
                    str[curpos] = ch;
                    curpos++;
                    curlen++;
                }
        }
        move (starty, startx + curpos);
    }
    str[curlen] = '\0';
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: endwin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
endwin (void)
{
    move (LINES - 1, 0);                                                        // move cursor to last line
    clrtoeol ();                                                                // clear this line
    mcurses_putc ('\017');                                                      // switch to G0 set
    curs_set (TRUE);                                                            // show cursor
    mcurses_puts_P(SEQ_REPLACE_MODE);                                            // reset insert mode
    refresh ();                                                                 // flush output
    mcurses_phyio_done ();                                                      // end of physical I/O
}
