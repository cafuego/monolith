typedef struct _menu_display_format_shit {
    char menu_title[100]; /* ""	title, if any. when setting, add a /n */
    int gen_0_idx;	  /* 0 	if true, display generates numeric index */
    int gen_1_idx; 	  /* 0  ditto, index starts at 1 instead of 0 */
    int no_boolean_values;/* 0 	values are not booleans */
    int auto_columnize;       /* unused, as of yet.  eek, eek */
	
    char text_color;		/* 'g' */
    char bracket_color;   	/* 'w' */
    char value_color;		/* 'r' */
    char index_color;		/* 'r' */

    char flag_char;  		/* '*' 	if values are booleans this is  */
				/*      the display char for "1" (on)   */
    char quit_command[20];      /* for quitting a menu */
    char menu_prompt[100];		/* command prompt for this menu, if any */

    /* internals, used by the code */
                 	/* effective max strlens, NOT a real strlen,  */
    int idx_emax;	/* but the strlen minus any colorcodes stored */
    int ival_emax;	/* as colorcodes don't take up display width */
    int txt_emax;
    int destroy_void_object;
 
    char menu_valid_input[200];  
 
    } M_display_t;

typedef struct _menu_item {
    char * MI_txt;	/* menu item text */
    char * MI_idx;	/* menu index, usually the command key */
    char * MI_ival;	/* menu index value-see M_display_t.no_boolean_values*/
    
    /* generic pointer to function */
    void (*vfunc)(const unsigned int, const long, void *);

    /* arguments to above */
    unsigned int ui_arg;
    long l_arg;
    void *obj_arg;

    } M_item_t;

typedef struct _menu_linked_list_node {
    struct _menu_linked_list_node *nextPtr;
    M_item_t *mi_ptr; 
    } M_LN_t, *M_LNptr;

/* prototypes */

extern M_LN_t *destroy_menu(M_LN_t *, M_display_t *);
extern M_LN_t * mono_display_menu_ified( M_LN_t *, M_display_t *, int);
extern M_LNptr add_menu_item(M_LNptr , void (*)(const unsigned int, const long, void *), unsigned int, long, void *, char *, ...);
extern void test_dlist_code(void);
extern void set_menu_defaults(M_display_t *);
extern void do_bing (const unsigned int, const long, void*);
extern void exec_menu_cmd(M_LN_t *, const char *);
extern void process_menu_internals(M_LN_t *, M_display_t *);
extern int menu_command(M_LN_t * , M_display_t * );
extern void do_nothing(const unsigned int, const long, void *);

/* macros */

#define MENU_DECLARE M_display_t the_menu_format; M_LN_t *the_menu = NULL
#define MENU_ADDITEM(a_vfunc, a_unsigned, a_long, a_voidP, format, arghs...)  \
        the_menu = add_menu_item(the_menu, a_vfunc, a_unsigned, a_long, a_voidP, format , ## arghs)
#define MENU_INIT set_menu_defaults(&the_menu_format)
#define MENU_PROCESS_INTERNALS \
	process_menu_internals(the_menu, &the_menu_format)
#define MENU_EXEC_COMMAND menu_command(the_menu, &the_menu_format)
#define MENU_DESTROY the_menu = destroy_menu(the_menu, &the_menu_format); \
            if (the_menu != NULL) \
                cprintf("\ndestroy_menu() fail.")
#define MENU_DISPLAY(COLUMNS) the_menu = \
             mono_display_menu_ified(the_menu, &the_menu_format, COLUMNS)

#define MALLOC_TMPSTR(x) tmpstr = (char *) xmalloc(sizeof(char) * x)
#define TMPSTR_STRCPY(x) strcpy(tmpstr, x)
#define MK_TMPSTR(x) MALLOC_TMPSTR(100); TMPSTR_STRCPY(x)

/* eof */


/*-----------------------------------
        documentation (sigh) 
------------------------------------*/
/* 
(disclaimer)
This thing is written with macros to make using it less of a pain in the
ass to use.  unfortunately, that's likely to make it a pain in the ass to use.

How to use:

To make a menu from anywhere:
-----------------------------------
1>
  somewhere in the variable declarations:  

MENU_DECLARE;  (declares the standard menu variables, etc.)
2> 
  wherever in the function you want the menu to display:

MENU_INIT; 
   sets menu formatting defaults..
3>
MENU_ADDITEM(function_name, functionarg, functionarg, functionarg, 
	     "tiv", "some text here..", "an index here", "a value here");
where function_name is the name of the function called by choosing that index, 
(if no function, use 'do_nothing' as defined in menu.c)

where the function args are:
the first arg is an unsigned int, the second arg is a long, the third arg is 
a void *.  you have to supply these, and prototype and define them in the 
function you want to call, even if they're not used.  yes, i know it's ugly.
for example:  to call function bing, defined somewhere as:
void bing(const unsigned int mask, const long unused, void * name)

MENU_ADDITEM(bing, 1024, 0, usersupp->username", 
	"tiv", usersupp->username, "n", (strlen(usersupp->username)) ? "1":"0");

for the user, at the exec prompt, pressing 'n' (the index) would execute:
	bing(1024,0,(char *) usersupp->username);

  (see notes on MENU_ADDITEM)

   use as many MENU_ADDITEM calls as needed, each item is added to a linked list

4>  override formatting defaults:

   customize any part of the_menu_format struct.  see typedef at top of header,
   and note the "defaults" in the comments, set by MENU_INIT.
   some things you'd might want to override:

the_menu_format.auto_columnize = 1;   (overrides any column value n passed to 
	DISPLAY(n), causes DISPLAY to figure out the number of cols on its own)

the_menu_format.quit_command (if you're exec-ing a menu function for this 
	menu, this array holds the characters that will quit the menu..  by 
	default this string is "\n\r "  (enter and space)  It will generate its
	own prompt for these, so all you have to do is adjust this string to
	whatever you want.  e.g. strcat, or for no quit at all, strcpy "" over 
	it.
the_menu_format.command_prompt  (again, if you're exec-ing a menu function for
	this menu, this is the command prompt.  i.e. if it's config_options
	menu, perhaps strcpy "Options Menu Command" over this.  it defaults to
	"Command")
strcpy(the_menu_format.menu_title, "\1f\1yBing's Bongs:\n"); (the menu title)

5>
MENU_PROCESS_INTERNALS;   sets up some stuff, mainly columnization crap.


6>
MENU_DISPLAY(n);
    displays the menu with n columns..  display code will handle from 1 to 
    4 columns, just replace n with whatever you like.  note:  if you turned
    the auto_columnize flag on, n is irrelevant, it'll use as many columns 
    as will display nicely.

7>  (optional)
MENU_EXEC_COMMAND;
    scans the index for valid_input, gets it, finds that input in the index,
    and executes the corresponding function.  returns an int, 1 if it executed
    something, 0 if not.  (useful for breaking out of loops)
i.e. 
    if (!MENU_EXEC_COMMAND)
	break;

8>
MENU_DESTROY;
    causes free() to be called a lot.  (:  also makes sure (hopefully) that
    any pointers left over are set to NULL

that's it.  see the notes below for a full explanation of what the
various members of _menu_display_format_shit do, and anything else i can
think of that might cause confusion.
---------------

Notes:
*****
MENU_ADDITEM:

..is bound to cause confusion.. there's a lot going on here.  but, by design
it all needs to be in the same statement, to ensure that if we use fp's, the
proper fp will be matched with the proper index.
the call is in 2 parts:  fp stuff, and item stuff.

fp stuff:  syntax is 'function_name, arg1, arg2, arg3'
where the function name is a function that:
  returns void,
  takes a const unsigned int parm,
  takes a const long parm,
  takes a const char * parm.

item stuff: it's a va_arg expansion macro of a 
va_arg function.  Think of it as a printf().  the first argument is the
format string, where: t stands for text, i stands for
index, and v stands for value.  they can be in any order, but they HAVE to 
correspond, much like a %s needs a string in printf(), and there can be at 
most 1 of each type in any given item..
 
in a single menu item there can be text, an index (which will display in 
brackets) and a value, which by default, displays a * if set to "1".
note that you can use any combination of these three, (or even none at all,
but that'll just display nothing..)  

for example, say we wanted a menu with a toggle bing item:  first, we need a
function that toggles items.  (this must be defined in the scope of the menu
code!  not necessarily in menu.[ch], but probably in the file you're creating
the menu from.  preferably, this will be a static function..)

anyways, say the example function to be called, as defined in somefile.c where
your're creating the menu:

void 
set_usersupp_flag(const unsigned int mask, const long unused,
                  const char *flagname)
{
    usersupp->flags ^= mask;
    cprintf("\n\1f\1c%s is now %s.\n", flagname, (usersupp->flags & mask) ?
                "\1gon" : "\1roff");
}

to build this into the menu, the call looks like this:
 
MENU_ADDITEM(set_usersupp_flag, US_BING, 0, "Bing", 
	     "tiv", "Toggle Bing", "a", "1");

   would wind up DISPLAY; -ing  something like this:

<a> * Toggle Bing

if you run MENU_EXEC_COMMAND after this, pressing 'a' would dereference the
fp to: set_usersupp_flag(US_BING, 0, "Bing")

note that the index and the value (as well as the text) is stored as a 
string, mainly for flexibility.

a more involved example:  say we wanted a V list, which showed the names
of the last few people who sent you an x, and you could send a reply x to 
them by numbered menu, note this is an example, it doesn't check for dups or
anything like that.  :b

...
the_menu_format.gen_1_idx = 1;
...
for (i = XLIMIT - 1; i >= 0 && xmsgb[i] && ctr < 5; i--) {
    MENU_ADDITEM(v_list_xmessage, 0, 0, x_mesgb[i]->sender,
	    "tv", x_mesgb[i]->sender,  (is_disabled) ? "1" : "0");
    ctr++;
}
...

DISPLAY(1);
...

<1> * Mad Man Murad
<2>   Cafuego
<3>   Kirth
<4> * Bing

then in the function v_list_xmessage, strcpy the char * parm over x_default, 
and do an express(-1).

************************************************ 

SET-ABLE "stuff" for customizing a menu.
--------
|  IMPORTANT! 
|
|MENU_DECLARE; 
|declares:    struct the_menu_format;
|
|MENU_INIT;
|initializes this struct to default values, via a call to: 
|	set_menu_display_defaults(&the_menu_format)  (see the macro defs above)
|
|sorry about the caveat, you're stuck with the struct name unless you want to
|do all that on your own, without the macros.
-------
ok now for the settable stuff:

	the_menu_format.menu_title;  (default = "")

strcpy a title here, or don't bother if you don't want a title displayed..

	the_menu_format.gen_0_idx  (default = 0  (off))

used to generate a numeric index, like for Peter's BBS configs..  as they
are chosen by relative position in shm, and subject to change, if one is
added, or deleted..   note that at the display function, if you tell it to
generate numbers AND you used your own indices, your indices will get
overwritten by the generated ones.
 
        the_menu_format.gen_1_idx  (default = 0  (off))

same as gen_0_index, but numbering starts with 1 rather than 0.
 
	the_menu_format.no_boolean_values  (default = 0 (off))

if left off: the "value" portion of the menu_item is interpreted at the display
    as a boolean..  i.e. if the value == "1" it will display a flag_char in that
    column, and if the value is anything else, it displays a space there.
if turned on:  the value portion of the menu is displayed "as is", i.e. if
the value is "231" it will display "231".
 
autocolumnize is available, and if on, overrides anything you pass to DISPLAY();
 
color is settable via the color members:

        the_menu_format.text_color  (default 'g')
        the_menu_format.bracket_color  (default 'w')
        the_menu_format.index_color  (default 'r')
        the_menu_format.value_color  (default 'r')

note that you can include any additional colorcodes in the text portion
of the menu item..  try to avoid that with the value and the index thou,
it'll prolly make the display look weird.
 
	the_menu_format.flag_char

the char that's displayed when all of these are true:  values are defined 
for the item, no_boolean_values = 0, and the actual value is "1".
 
---------------------------
how it works.
(rather, how i think it works)

the data structure:  
 
there are two basic root types:  the first is a linked list, that holds 
a list of "menu items":

	malloc'd LINKED LIST.
	pointer to next node in LINKED LIST, and pointer to DATA.

	DATA:  malloc'd too, consists of pointers to malloc'd CHAR arrays,
		a function pointer, and the function pointed to's parameters.
		
************************************************************************
     everything documented after this point is correct, but dated, before
     function pointers were added.
**********************************************************************

the second is a struct that defines various things about the menu.

both are passed around like joints via pointers.

the functions, in normal order of call:

	extern void test_dlist_code(void);
uninteresting test of "d list code", probably an example of sorts.

	extern M_LNptr add_menu_item(M_LNptr , char *, ...);
calls init_menu_item() to ensure everything is properly NULL, 
handles allocation of various char arrays, sifts the args into the 
proper arrays, and calls linked_list_insertNode() to add the item
to the menu.

	static M_LNptr linked_list_insertNode(M_LNptr, M_item_t *);
allocates a node, adds a menu item to that node, adds node to the linked list

	extern void set_menu_display_defaults(M_display_t *);
initializes menu-wide struct to default values for a generic menu.

	extern M_LN_t * mono_display_menu_ified( M_LN_t *, M_display_t *, int);
does far too much to be a single function.  (:

	extern M_LN_t *destroy_menu(M_LN_t *);
free()'s everything, sets any remaining pointers to NULL.

these four functions are used by mono_display_menu_ified():

static int linked_list_itemtotal(M_LN_t *);
	counts the items in the linked list

static M_LN_t *column_sort_linked_list(M_LN_t *, int);
	creates another linked list, copies pointers to items in 
	columnized order, and destroys the source list.  (not the data)

static int cnt_cc(char *);
	counts the colorcodes in a string by doubling '\1' characters, 
	usually subtracted from a strlen of that string, or anything 
	else that's lying about helpless..

static void set_subcolumn_emax(M_display_t *, M_LN_t *);
	determines the "effective display strlen" of the value array, 
	and the index array, and stores that in the_menu_format.ival_emax
        and the_menu_format.idx_emax respectively.  used to calculate 
	column spacing, and space-padding.

*/
