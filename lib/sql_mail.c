/* $Id$ */
/* operatoins we can do on users */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_MYSQL_H
  #undef HAVE_MYSQL_MYSQL_H
  #include <mysql.h>
#else
  #ifdef HAVE_MYSQL_MYSQL_H
    #undef HAVE_MYSQL_H
    #include <mysql/mysql.h>
  #endif
#endif

#include "monolith.h"
#include "monosql.h"
#include "sql_mail.h"
#include "sql_utils.h"

int
mono_sql_add_mail()
{

    return 0;
}

int
mono_sql_remove_mail(int mail_id)
{

    return 0;
}

int
mono_sql_retrieve_mail(int mail_id)
{

    return 0;
}


/* eof */
