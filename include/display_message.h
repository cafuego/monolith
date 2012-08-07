/*
 * $Id$
 *
 * protos for src/display_message.c
 */

/* prototypes */

int display_message(unsigned int, unsigned int, const unsigned int);
void display_message_header(message_header_t *);
unsigned int message_reply_id(unsigned int);
char * message_reply_name(char *);
void show_long_prompt(const unsigned int, const unsigned int, const int);

#ifdef MERGE_CODE_FOR_THE_RECORD
void display_message(message_t *message);
void display_header(message_t *message);
#endif
