<?

 /*
  * $Id$
  */

function express_message($username, $recipient) {

    // Turn recipient name into a normal string.
    //
    $recipient = rawurldecode($recipient);

    // Guests aren't allowed to X.
    //
    if( $username == "Guest" ) {
        print "<h3 align=\"center\">Guests are not allowed to send eXpress messages.</h3>\n";
        return;
    }
    if ($recipient == "Guest" ) {
        print "<h3 align=\"center\">You cannot send eXpress messages to guests.</h3>\n";
        return;
    }

    // Check user priv.
    //
    switch(check_friend($username,$recipient)) {

        case -4:
            print "<h3 align=\"center\">$recipient has web express messages disabled.</h3>\n";
            return;
        case -3:
            print "<h3 align=\"center\">$recipient is not currently online.</h3>\n";
            return;
        case -2:
            print "<h3 align=\"center\">$recipient is not an exisiting user.</h3>\n";
            return;
        case -1:
            print "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
            return;
        case 0:
            print "<h3 align=\"center\">You are not allowed to send $recipient messages.</h3>\n";
            return;
        default:
            break;
    }

    print "<h3 align=\"center\">Sending an eXpress message to $recipient</h3>\n";
    print "<h3 align=\"center\">.. but not quite yet. We're working on it, though.</h3>\n";
    if($username != "Cafuego")
    return;

    $recp_id = get_usernum($recipient);

    // Show message.
    //
    PRINT "<div style=\"margin-left: 150;\">\n";

    // Reply form.
    //
    PRINT "<form action=\"/bbs/sendx.phtml\" method=\"post\">\n";

    // Hidden but required info for reply.
    //
    PRINT "<input type=\"hidden\" name=\"recipient\" value=\"$recp_id\">\n";

    // Show formatted header.
    //
    $now = strftime("%H:%M", time());
    PRINT "<p align=\"left\"><font color=\"#00CC00\">eXpress Message from</font> <font color=\"#ffff00\">$username</font> <font color=\"#00cc00\">to</font> <font color=\"#ffff00\">$recpient</font> <font color=\"#00cc00\">at</font> (<font color=\"#00cc00\">$now</font>)</p>\n";
    PRINT "<textarea rows=\"6\" cols=\"70\" name=\"message\" style=\"font: 10pt Helvetica\">Enter your message here.</textarea>\n";

    // Form buttons.
    //
    PRINT "<input type=\"submit\" value=\"Send X!\">\n";
    PRINT "<input type=\"reset\" value=\"Clear\">\n";

    PRINT "</form>\n";
    PRINT "</div>\n";

}

?>
