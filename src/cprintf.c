/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * Code for cprintf(), done properly, just like KH would really like.
 * I don't pretend to understand anything here, I've just adapted it,
 * as needed, from:
 */

/*
 *  linux/kernel/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * I imagine that Linus Torvalds would appreciate that being left here...
 * Have fun...  -  -  Diamond White Dave.
 */

/*
 * vsprintf.c -- Lars Wirzenius & Linus Torvalds. 
 */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

int bing(int c);
void number(long num, int base, int size, int precision, int type);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
int vcprintf(const char *fmt, va_list args);
int cprintf(const char *fmt,...);
extern void ColourChar(char key);

/* quick hack added by michel to send out an CR+LF in stead of just a LF */
int
bing(int c)
{
    if ( c == '\n')
	putchar('\r');
    return putchar(c);
}

unsigned long
simple_strtoul(const char *cp, char **endp, unsigned int base)
{
    unsigned long result = 0, value;

    if (!base) {
	base = 10;
	if (*cp == '0') {
	    base = 8;
	    cp++;
	    if ((*cp == 'x') && isxdigit(cp[1])) {
		cp++;
		base = 16;
	    }
	}
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : (islower(*cp)
				 ? toupper(*cp) : *cp) - 'A' + 10) < base) {
	result = result * base + value;
	cp++;
    }
    if (endp)
	*endp = (char *) cp;
    return result;
}

/*
 * we use this so that we can do without the ctype library 
 */
#define is_digit(c)     ((c) >= '0' && (c) <= '9')

static int
skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s))
	i = i * 10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1		/*
				 * * * * * * * pad with zero  
				 */
#define SIGN    2		/*
				 * * * * * * * unsigned/signed long  
				 */
#define PLUS    4		/*
				 * * * * * * * show plus  
				 */
#define SPACE   8		/*
				 * * * * * * * space if plus  
				 */
#define LEFT    16		/*
				 * * * * * * * left justified  
				 */
#define SPECIAL 32		/*
				 * * * * * * * 0x  
				 */
#define LARGE   64		/*
				 * * * * * * * use 'ABCDEF' instead of 'abcdef'  
				 */

#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

void
number(long num, int base, int size, int precision
       ,int type)
{
    char c, sign, tmp[36];
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
	digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
	type &= ~ZEROPAD;
    if (base < 2 || base > 36)
	return;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
	if (num < 0) {
	    sign = '-';
	    num = -num;
	    size--;
	} else if (type & PLUS) {
	    sign = '+';
	    size--;
	} else if (type & SPACE) {
	    sign = ' ';
	    size--;
	}
    }
    if (type & SPECIAL) {
	if (base == 16)
	    size -= 2;
	else if (base == 8)
	    size--;
    }
    i = 0;
    if (num == 0)
	tmp[i++] = '0';
    else
	while (num != 0)
	    tmp[i++] = digits[do_div(num, base)];
    if (i > precision)
	precision = i;
    size -= precision;
    if (!(type & (ZEROPAD + LEFT)))
	while (size-- > 0)
	    putchar(' ');
    if (sign)
	putchar(sign);
    if (type & SPECIAL) {
	if (base == 8)
	    putchar('0');
	else if (base == 16) {
	    putchar('0');
	    putchar(digits[33]);
	}
    }
    if (!(type & LEFT))
	while (size-- > 0)
	    bing(c);
    while (i < precision--)
	putchar('0');
    while (i-- > 0)
	bing(tmp[i]);
    while (size-- > 0)
	putchar(' ');
    return;
}

int
vcprintf(const char *fmt, va_list args)
{
    int len, DynamicColourFlag = 0, ColourCount;
    unsigned long num;
    int i, base;
    char *s;

    int flags;			/* flags to number() */
    int field_width;		/* width of output field */
    int precision;		/* min. # of digits for integers; max
				 * number of chars for from string 
				 */
    int qualifier;		/* 'h', 'l', or 'L' for integer fields */

    for (; *fmt; ++fmt) {
	ColourCount = 0;
	if (*fmt != '%') {
	    if (*fmt == 1) {
		if (*(fmt + 1) == '%') {
		    DynamicColourFlag = 1;	/*
						 * Tricky stuff... 
						 */
		} else {
		    DynamicColourFlag = 0;
		    ColourChar(*(++fmt));
		}
	    } else
		bing(*fmt);
	    continue;
	}
	/*
	 * process flags 
	 */
	flags = 0;
      repeat:
	++fmt;			/*
				 * this also skips first '%' 
				 */
	switch (*fmt) {
	    case '-':
		flags |= LEFT;
		goto repeat;
	    case '+':
		flags |= PLUS;
		goto repeat;
	    case ' ':
		flags |= SPACE;
		goto repeat;
	    case '#':
		flags |= SPECIAL;
		goto repeat;
	    case '0':
		flags |= ZEROPAD;
		goto repeat;
	}

	/*
	 * get field width 
	 */
	field_width = -1;
	if (is_digit(*fmt))
	    field_width = skip_atoi(&fmt);
	else if (*fmt == '*') {
	    ++fmt;
	    /*
	     * it's the next argument 
	     */
	    field_width = va_arg(args, int);
	    if (field_width < 0) {
		field_width = -field_width;
		flags |= LEFT;
	    }
	}
	/*
	 * get the precision 
	 */
	precision = -1;
	if (*fmt == '.') {
	    ++fmt;
	    if (is_digit(*fmt))
		precision = skip_atoi(&fmt);
	    else if (*fmt == '*') {
		++fmt;
		/*
		 * it's the next argument 
		 */
		precision = va_arg(args, int);
	    }
	    if (precision < 0)
		precision = 0;
	}
	/*
	 * get the conversion qualifier 
	 */
	qualifier = -1;
	if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
	    qualifier = *fmt;
	    ++fmt;
	}
	/*
	 * default base 
	 */
	base = 10;

	switch (*fmt) {
	    case 'c':
		if (!(flags & LEFT))
		    while (--field_width > 0)
			putchar(' ');
		if (DynamicColourFlag) {
		    DynamicColourFlag = 0;
		    ColourChar((unsigned char) va_arg(args, int));
		} else
		    bing((unsigned char) va_arg(args, int));
		while (--field_width > 0)
		    putchar(' ');
		continue;

	    case 's':
		s = va_arg(args, char *);
		if (!s)
		    s = "\nAUGH, he cried in true KH style.\nYell, right now, and tell us what you pressed.\n";
		len = strlen(s);
		if (precision < 0)
		    precision = len;
		else if (len > precision)
		    len = precision;

		if (!(flags & LEFT))
		    while (len < field_width--)
			putchar(' ');
		for (i = 0; i < len; ++i) {
		    if (!*s) {
			i = len;
			break;
		    }
		    if (*s == 1) {
			ColourCount++;
			s++;	/* * Skip over ^A */
			ColourChar(*s);		/*
						 * Convert the Colour character 
						 */
			s++;	/*
				 * Move on. 
				 */
			i--;	/*
				 * Rewind i, after all, we haven't printed 
				 * anything. 
				 */
		    } else
			bing(*s++);	/*
					 * Copy over 
					 */
		}
		while (len < (field_width-- + (ColourCount * 2)))
		    putchar(' ');
		continue;

	    case 'p':
		if (field_width == -1) {
		    field_width = 2 * sizeof(void *);
		    flags |= ZEROPAD;
		}
		number((unsigned long) va_arg(args, void *), 16,
		       field_width, precision, flags);
		continue;


/*
 * case 'n': if (qualifier == 'l') { long * ip = va_arg(args, long *); *ip 
 * = (str - buf); } else { int * ip = va_arg(args, int *); *ip = (str -
 * buf); } continue; * I don't support %n: I cannot actually see the point 
 * of it. * Feel free to put one in. Just count up however many *
 * bing()'s vcprintf and number use, and return that. 
 */
		/*
		 * integer number formats - set up the flags and "break" 
		 */
	    case 'o':
		base = 8;
		break;

	    case 'X':
		flags |= LARGE;
	    case 'x':
		base = 16;
		break;

	    case 'd':
	    case 'i':
		flags |= SIGN;
	    case 'u':
		break;

	    default:
		if (*fmt != '%')
		    putchar('%');
		if (*fmt)
		    bing(*fmt);
		else
		    --fmt;
		continue;
	}
	if (qualifier == 'l')
	    num = va_arg(args, unsigned long);
	else if (qualifier == 'h')
	    if (flags & SIGN)
		num = va_arg(args, int);
	    else
		num = va_arg(args, int);
	else if (flags & SIGN)
	    num = va_arg(args, int);
	else
	    num = va_arg(args, unsigned int);
	number(num, base, field_width, precision, flags);
    }
    return (0);
}

int
cprintf(const char *fmt,...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vcprintf(fmt, args);
    va_end(args);
    return i;
}

/* EOF */
