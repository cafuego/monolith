<?

 /*
  * $Id$
  */

function express_message($username, $recipient) {

    // Turn recipient name into a normal string.
    //
    print "<p>Checking user '$recipient' -&gt; ";
    $recipient = rawurldecode($recipient);
    $recipient = strtr($recipient, "+", " ");
    print "'$recipient'.\n";

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

    connect_db();

}

?>
