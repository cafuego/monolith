/* Banana, the name game */
/* banana.c v1.2  --  idea Sven Heinicke */
/*                    code Sven Heinicke and Dave Becker */
/*                    {sven, becker}@wells.nrl.navy.mil */

/* Special thanks goes out to a Mr. Stainless Steel Rat */
/* <ratinox@ccs.neu.edu> (I guess Stainless is a Mr. and not a Ms.) */
/* for information on how to handel these bad words. */

/* Mon Feb 28 15:50:41 EST 1994 -- it now checks for bad words. */

#include <stdio.h>
#include <string.h>

/* bad words to look for: Leave out the first letter and seporate them */
/* by ":"s. */
#define B_WORDS "itch:ITCH"
#define F_WORDS "uck:UCK"
#define M_WORDS ""

/* What to use instead of those bad words. */
#define REPLACEMENT "arles"

/* just in case you want to change your vowels.  Do vowels change? */
#define VOWELS "AaEeIiOoUuy"

/* make it so you can integrate banana in your own programs. */
int banana ();
/* bad word check function. */
const char *word_check();

/* the fruit and vegetables of the program. */
int
banana (name)
     char *name;     /* the name to be name game'd. */
{
  char
    *first_vowel,
    *no_vowel_check,
    *strchr_tmp,
    *vowel_pointer = VOWELS;

  /* point to the ending NULL of name. */
  no_vowel_check = first_vowel = &name[strlen(name) + 1];
  /* loop until we go through all the vowels. */
  while (*vowel_pointer != (char)NULL)
    {
      /* find the first vowel. */
      strchr_tmp = strchr (name, *vowel_pointer);
      /* check if it is the first known vowel in the name. */
      if ((strchr_tmp != NULL) && (strchr_tmp < first_vowel))
	{
	  /* if it is the first know vowel in the name, remember it. */
	  first_vowel = strchr_tmp;
	}
      /* go and check the next vowel. */
      ++vowel_pointer;
    }
  /* make sure the name had some vowels at all. */
  if (first_vowel == no_vowel_check)
    {
      /* if it did not find any vowels, get confused. */
      fprintf (stderr, "\"%s\" don't got any vowels.  I'm confused.\n",
	       name);
      /* and error out. */
      return 1;
    }
  /* finally, print the song, and check for bad words. */
  printf ("\n%s %s Bo B%s\nBanana Fana Fo F%s\nMi My Mo M%s\n%s\n\n",
	  name, name, word_check (first_vowel, B_WORDS, REPLACEMENT),
	  word_check (first_vowel, F_WORDS, REPLACEMENT),
	  word_check (first_vowel, M_WORDS, REPLACEMENT),
	  name);
  /* leave nicely. */
  return 0;
}

/* function that compares unknows_word with word tokens seporated by */
/* ":" in bad_word_list.  If it finds no matches it returns */
/* unknows_word, if it finds a match it returns replacement. */
const char *
word_check (unknown_word, bad_word_list, replacement)
     char *unknown_word;        /* word to check. */
     const char *bad_word_list; /* list of bad words */
     const char *replacement;   /* word to return if unknow_word is a */
				/* bad word. */
{
  char
    *a_bad_word,           /* Points to bad words down */
			   /* bad_word_list_copy. */
    *bad_word_list_copy;   /* because strtok() changes its argument, */
			   /* we need to make a copy of bdc_word_list */
			   /* before we use it. */

  /* copy bad_word_list into bad_word_list_copy. */
  strcpy (bad_word_list_copy = (char *)malloc (sizeof (bad_word_list)),
	  bad_word_list);
  /* point to the first token in bad_word_list_copy. */
  a_bad_word = strtok (bad_word_list_copy, ":");
  /* stop if we got no bad words in the list, or if we just checked */
  /* the last bad word. */
  while (a_bad_word != (char*)NULL)
    {
      /* compare a_bad_word with the unknown_word. */
      if (strcmp (a_bad_word, unknown_word) == 0)
	{
	  /* free up the memory used by bad_word_list_copy. */
	  free (bad_word_list_copy);
	  /* if they are equal return the replacement. */
	  return replacement;
	}
      /* go get the next bad word on the list. */
      a_bad_word = strtok ((char*)NULL, ":");
    }
  /* free up the memory used by bad_word_list_copy. */
  free (bad_word_list_copy);
  /* return the unknown word. */
  return unknown_word;
}
