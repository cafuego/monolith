/*************************************************
* hprintf()
* Function to convert color codes to HTML format and convert characters
* with special meaning in HTML to a coded format
* 					1996, Xar
* changed a bit by Kirth
*************************************************/

#include <stdio.h>

#include "cgi.h"

int special_char(char);
int color_code(char);

#define DEFAULT 0x1000000

void
hprintf(const char *string)
{
    const register char *p;
    static int news = DEFAULT, old = DEFAULT;

    p = string;
    do {
	if (*p == 1) {		/* CTRL-A */
	    p++;
	    news = color_code(*p);
	    if (old != news) {	/*  color changed!! */
		if (old == DEFAULT)
		    printf("<font color=#%06x>", news);
		else if (news == DEFAULT)
		    printf("</font>");
		else
		    printf("</font><font color=#%06x>", news);
	    }
	    old = news;
	} else {
	    if (!special_char(*p))
		printf("%c", *p);
	}
    }
    while (*p++);
    return;
}

int
color_code(char c)
{
    int news;

    switch (c) {
	case 'a':
	    news = DEFAULT;
	    break;
	case 'r':		/* red */
	    news = 0xff0000;
	    break;
	case 'g':		/* green */
	    news = 0x00ff00;
	    break;
	case 'y':		/* yellow */
	    news = 0xffff00;
	    break;
	case 'b':		/* blue */
	    news = 0x0000ff;
	    break;
	case 'p':		/* purple */
	    news = 0xff00ff;
	    break;
	case 'c':		/* cyan */
	    news = 0x00ffff;
	    break;
	case 'w':		/* white */
	    news = 0xffffff;
	    break;
	default:
	    news = 0x00ff00;
	    break;
    }

    return news;
}

int
special_char(char c)
{
    switch (c) {
	case 0:
	    return 1;
	case 13:
	    printf("\n");
	    return 1;
	case '<':
	    printf("&lt;");
	    return 1;
	case '>':
	    printf("&gt;");
	    return 1;
	case '&':
	    printf("&amp;");
	    return 1;
	case '"':
	    printf("&quot;");
	    return 1;
    }
    return 0;
}
