/* $Id$ */

typedef struct rbbs_type {
    char name[L_BBSNAME + 1];
    unsigned int port;
    char addr[16];
} rbbs_t;

/* dexi_routines.c */

void mono_setdexient(void);
void mono_enddexient(void);
rbbs_t *mono_getdexient(void);

/* eof */
