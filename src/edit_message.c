#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <build-defs.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>

#ifdef USE_MYSQL
#include MYSQL_HEADER
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include "monolith.h"
#include "libmono.h"
#include "sql_message.h"

#include "ext.h"
#include "input.h"

#include "main.h"
#include "messages.h"
#include "rooms.h"
#include "routines2.h"

#define extern
#include "edit_message.h"
#undef extern

int
edit_message(unsigned int forum, unsigned int id)
{
    char msgfile[L_FILENAME+1], hdrfile[L_FILENAME+1], tmpfile[L_FILENAME+1];
    message_header_t *header;
    room_t quad;

    nox = TRUE;

    read_forum(forum, &quad);

    strcpy(msgfile, "");
    sprintf(msgfile, "%s/%u/%u.t", FORUMDIR, forum, id );

    strcpy(hdrfile, "");
    sprintf(hdrfile, "%s/%u/%u.h", FORUMDIR, forum, id );

    strcpy(tmpfile, "");
    sprintf(tmpfile, "%s/tmp/%s.edit.%d", BBSDIR, usersupp->username, getpid() );

    if(fexists(temp))
        unlink(temp);

    if (copy(msgfile, temp) != 0)
        return -1;

    editor_edit(temp);

    header = (message_header_t *) xmalloc(sizeof(message_header_t));
    init_message_header(header);

    // Read and update the header info.
    (void) read_message_header(hdrfile, header);
    header->orig_m_id = 42;
    header->orig_date = header->date;
    header->date = time(NULL);
    header->mod_type = MOD_EDIT;
    strcpy(header->modified_by, usersupp->username);

    // Save the original post to temp file.
    if (copy(msgfile, tmpfile) != 0) {
        xfree(header);
        log_it("errors", "Failed to create a temporary backup of message %u in forum %d", id, forum);
        cprintf("\1f\1rCould not create temporary backup of message. Error logged.\n");
        return -1;
    }

    // Remove current message file.
    if(fexists(msgfile))
        unlink(msgfile);
    
    // Copy editor file to message file.
    if (copy(temp, msgfile) != 0) {
        xfree(header);
        log_it("errors", "Failed saving edited message %u in forum %d", id, forum);
        cprintf("\1f\1rCould not save edited message. Error logged.\n");

        // Try to copy the backup back if it fails.
        if(copy(tmpfile, msgfile) != 0) {
            log_it("errors", "Failed trying to restore unedited message %u in forum %u from backup", id, forum);
            cprintf("\1f\1rCould not restore saved message from backup. Error logged.\n");
            return -3;
        }
        return -2;
    }

    // Save the updated header.
    if(fexists(hdrfile));
        unlink(hdrfile);

    if(  write_message_header(hdrfile, header) != 0 ) {
        log_it("errors", "Failed trying to update header for message %u in forum %u", id, forum);
        cprintf("\1f\1rFailed to save updated message header. Error logged.\n");
    }

    // Clean up.
    if(fexists(tmpfile));
        unlink(tmpfile);
    xfree(header);

    log_it("editlog", "%s edited message %u in forum %u.%s", usersupp->username, id, forum, quad.name);
    cprintf("\n\1f\1f\1gEdited message was saved in %s successfully.\n");

    return 0;
}
