/*
 * $Id$
 *
 * Global JavaScript functions for BBS Web Interface.
 */

function logout( url ) {

    // Check where we're going.
    // If not to a link on the BBS pages, then we go and
    // visit our logout page.
    //
    if( url.indexOf("/bbs/") == -1 )
        open("/bbs/index.phtml/q");

}

function setStatus( text ) {

    // Set status window content to `text'.
    //
    window.status = text;
    return true;

}
