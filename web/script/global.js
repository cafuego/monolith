/*
 * $Id$
 *
 * Global JavaScript functions for BBS Web Interface.
 */

function logout() {

    // Check where we're going.
    // If not to a link on the BBS pages, then we go and
    // visit our logout page.
    //
    alert(window.location);
    if( window.location.indexOf('/bbs/') == -1 ) {
        confirm("You are leaving Monolith BBS\n\nYou must log out first and will\nnow be taken to the logout page.");
        self.location = "/bbs/index.phtml/q";
    }

}

function setStatus( text ) {

    // Set status window content to `text'.
    //
    window.status = text;
    return true;

}
