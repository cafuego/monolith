/*
 * $Id$
 *
 * Global JavaScript functions for BBS Web Interface.
 */

function logout() {

    open("/bbs/index.phtml/q");

}

function setStatus( text ) {

    window.status = text;
    return true;

}
