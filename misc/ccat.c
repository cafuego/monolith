/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#ifdef HAVE_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#define BUFSIZE 1024

int main(int argc, char *argv[]);
int bing(int c);
char *getcolour(char col);
void ColourChar(char key);
void number(long num, int base, int size, int precision, int type);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
int vcprintf(const char *fmt, va_list args);
int cprintf(const char *fmt,...);

char CURRCOL = '\0';

struct option longopts[] =
{
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

static char *progname;

int
main(int argc, char *argv[])
{

    FILE *fp = NULL;
    char buf[BUFSIZE];
    long lines = 0;
    int h = 0, v = 0;
    int optc;
    int lose = 0;



#ifdef HAVE_GETTEXT
       setlocale (LC_ALL, "");
       bindtextdomain ("monolith", "/usr/bbs/lib/locale");
       textdomain ("monolith");
#endif



    progname = argv[0];

    while ((optc = getopt_long(argc, argv, "hv", longopts, (int *) 0))
	   != EOF)
	switch (optc) {
	    case 'v':
		v = 1;
		break;
	    case 'h':
		h = 1;
		break;
	    case '?':
		lose = 1;
		break;
	    default:
		lose = 1;
		break;
	}

    if (lose || optind < argc - 1) {
	/* Print error message and exit.  */
	if (optind < argc)
	    fputs(_("Too many arguments\n"), stderr);
	fprintf(stderr, _("Try `%s --help' for more information\n"),
		progname);
	exit(1);
    }
    /* `help' should come first.  If `help' is requested, ignore the other
     * options. */
    if (h) {
	/* Print help info and exit.  */
	printf(_("\
This is Monolith ccat.\n\
Usage: %s [OPTION] [FILE]\n\
  -h, --help          display this help and exit\n\
  -v, --version       display version information and exit\n\
\n\
Report bugs to bugs@monolith.yawc.net.\n"),
	       progname);

	exit(0);
    }
    if (v) {
	/* Print version number.  */
	printf("ccat - %s %s\n", "Monolith", "1.0");
	exit(0);
    } else {

	if (optind == argc) {
	    fp = stdin;
	} else {
	    if ((fp = fopen(argv[optind], "r")) == NULL) {
		fprintf(stderr, "%s: %s: %s\n", progname, argv[optind], strerror(errno));
		return 1;
	    }
	}
	while (fgets(buf, BUFSIZE, fp) != NULL) {
	    buf[strlen(buf) - 1] = '\0';
	    cprintf("%s\n", buf);
	    lines++;
	}

	if (CURRCOL != 'a' && CURRCOL != '\0') {
	    fprintf(stderr, _("Warning: File ends in colour `%s'. Please fix this\n"), getcolour(CURRCOL));
	    cprintf("\1a");
	}
/*    fprintf(stderr, _("[%s: %ld line%s]\n"), argv[optind], lines, (lines == 1) ? "" : "s"); */
	fclose(fp);
	return 0;
    }

}

int
bing(int c)
{
    if (c == '\n')
	putchar('\r');
    return putchar(c);
}

char *
getcolour(char col)
{

    col = tolower(col);

    switch (col) {
	case 'd':
	    return "dark";
	    break;
	case 'r':
	    return "red";
	    break;
	case 'g':
	    return "green";
	    break;
	case 'y':
	    return "yellow";
	    break;
	case 'b':
	    return "blue";
	    break;
	case 'p':
	    return "purple";
	    break;
	case 'c':
	    return "cyan";
	    break;
	case 'w':
	    return "white";
	    break;
	case 'f':
	    return "bold";
	    break;
	case 'e':
	    return "blink";
	    break;
	case 'i':
	    return "inverse";
	    break;
	case 'h':
	    return "hidden";
	    break;
	default:
	    break;

    }
    return NULL;
}

void
ColourChar(char key)
{
    switch (key) {
	case 'd':
	    cprintf("[30m");
	    break;		/* dark   textcolor       */
	case 'r':
	    cprintf("[31m");
	    break;		/* red    textcolor       */
	case 'g':
	    cprintf("[32m");
	    break;		/* green  textcolor       */
	case 'y':
	    cprintf("[33m");
	    break;		/* yellow textcolor       */
	case 'b':
	    cprintf("[34m");
	    break;		/* blue   textcolor       */
	case 'p':
	    cprintf("[35m");
	    break;		/* purple textcolor       */
	case 'c':
	    cprintf("[36m");
	    break;		/* cyan   textcolor       */
	case 'w':
	    cprintf("[37m");
	    break;		/* white  textcolor       */
	case 'D':
	    cprintf("[40m");
	    break;		/* dark   backgroundcolor */
	case 'R':
	    cprintf("[41m");
	    break;		/* red    backgroundcolor */
	case 'G':
	    cprintf("[42m");
	    break;		/* green  backgroundcolor */
	case 'Y':
	    cprintf("[43m");
	    break;		/* yellow backgroundcolor */
	case 'B':
	    cprintf("[44m");
	    break;		/* blue   backgroundcolor */
	case 'P':
	    cprintf("[45m");
	    break;		/* purple backgroundcolor */
	case 'C':
	    cprintf("[46m");
	    break;		/* cyan   backgroundcolor */
	case 'W':
	    cprintf("[47m");
	    break;		/* white  backgroundcolor */
	case 'a':
	    cprintf("[0m");
	    break;		/* RESET   attribute      */
	case 'f':
	    cprintf("[1m");
	    break;		/* BOLD    attribute      */
	case 'u':
	    cprintf("[4m");
	    break;		/* UNDERLINED attribute   */
	case 'e':
	    cprintf("[5m");
	    break;		/* FLASH   attribute      */
	case 'i':
	    cprintf("[7m");
	    break;		/* INVERSE attribute      */
	case 'h':
	    cprintf("[8m");
	    break;		/* HIDDEN  attribute      */
	default:
	    break;
    }
    CURRCOL = key;
    return;
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
				 * * * * * * * * * * * pad with zero  
				 */
#define SIGN    2		/*
				 * * * * * * * * * * * unsigned/signed long  
				 */
#define PLUS    4		/*
				 * * * * * * * * * * * show plus  
				 */
#define SPACE   8		/*
				 * * * * * * * * * * * space if plus  
				 */
#define LEFT    16		/*
				 * * * * * * * * * * * left justified  
				 */
#define SPECIAL 32		/*
				 * * * * * * * * * * * 0x  
				 */
#define LARGE   64		/*
				 * * * * * * * * * * * use 'ABCDEF' instead of 'abcdef'  
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
	    bing(' ');
    if (sign)
	bing(sign);
    if (type & SPECIAL) {
	if (base == 8) {
	    bing('0');
	} else if (base == 16) {
	    bing('0');
	    bing(digits[33]);
	}
    }
    if (!(type & LEFT))
	while (size-- > 0)
	    bing(c);
    while (i < precision--)
	bing('0');
    while (i-- > 0)
	bing(tmp[i]);
    while (size-- > 0)
	bing(' ');
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
			bing(' ');
		if (DynamicColourFlag) {
		    DynamicColourFlag = 0;
		    ColourChar((unsigned char) va_arg(args, int));
		} else
		    bing((unsigned char) va_arg(args, char));
		while (--field_width > 0)
		    bing(' ');
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
			bing(' ');
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
		    bing(' ');
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
		    bing('%');
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
		num = va_arg(args, short);
	    else
		num = va_arg(args, unsigned short);
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
