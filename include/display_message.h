/*
 * $Id$
 *
 * protos for src/display_message.c
 */

/* prototypes */

extern int display_message(unsigned int, unsigned int, const unsigned int);
extern void display_message_header(message_header_t *);
extern unsigned int message_reply_id(unsigned int);
extern char * message_reply_name(char *);
extern void show_long_prompt(const unsigned int, const unsigned int, const int);

#ifdef MERGE_CODE_FOR_THE_RECORD
extern void display_message(message_t *message);
extern void display_header(message_t *message);
#endif
