<?

 /*
  * $Id$
  */

function express_message($username, $recipient) {

    // Turn recipient name into a normal string.
    //
    $recipient = rawurldecode($recipient);
    $recipient = strtr($recipient, "+", " ");

    // Guests aren't allowed to X.
    //
    if( $username == "Guest" ) {
        print "<h3 align=\"center\">Guests are not allowed to send eXpress messages.</h3>\n";
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

}

?>
