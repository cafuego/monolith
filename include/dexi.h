/* $Id$ */

typedef struct rbbs_type {
    char name[L_BBSNAME + 1];
    unsigned int port;
    char addr[16];
} rbbs_t;

/* dexi_routines.c */

extern void mono_setdexient(void);
extern void mono_enddexient(void);
extern rbbs_t *mono_getdexient(void);

/* eof */
