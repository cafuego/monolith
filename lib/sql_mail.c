/* $Id$ */
/* operatoins we can do on users */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <build-defs.h>
#include <stdio.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
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
