<?

 /*
  * $Id$
  */

function express_message($username, $recipient, $interface, $location)
{

    // Turn recipient name into a normal string.
    //
    $recipient = rawurldecode($recipient);

    // Guests aren't allowed to X.
    //
    if ($username == "Guest") {
	sorrysound();
	errorimage("Guests are not allowed to send eXpress messages!");
	return;
    }
    if ($recipient == "Guest") {
	sorrysound();
	errorimage("You cannot send eXpress messages to guests!");
	return;
    }
    // Check user priv.
    //
    switch (check_friend($username, $recipient)) {

	case -4:
	    sorrysound();
	    errorimage("$recipient has web express messages disabled!");
	    return;
	case -3:
	    sorrysound();
	    errorimage("$recipient is not currently online!");
	    return;
	case -2:
	    errorsound();
	    errorimage("$recipient is not an exisiting user!");
	    return;
	case -1:
	    errorsound();
	    errorimage("You are not currently logged in. Log in first!");
	    return;
	case 0:
	    sorrysound();
	    errorimage("You are not allowed to send $recipient messages");
	    return;
	default:
	    break;
    }

    addimage("Sending an eXpress message to $recipient@$interface");

    // Reply form.
    //
    echo "<form action=\"/bbs/handler.phtml\" method=\"post\">\n";

    // Hidden but required info for reply.
    //
    echo "<input type=\"hidden\" name=\"handler\" value=\"x\">\n";
    echo "<input type=\"hidden\" name=\"recipient\" value=\"$recipient\">\n";
    echo "<input type=\"hidden\" name=\"sender\" value=\"$username\">\n";
    echo "<input type=\"hidden\" name=\"interface\" value=\"$interface\">\n";
    echo "<input type=\"hidden\" name=\"location\" value=\"$location\">\n";

    // Show formatted header.
    //
    $now = strftime("%H:%M", time());
    echo "<p align=\"left\"><font color=\"#00CC00\">eXpress Message from</font> <font color=\"#ffff00\">$username@web</font> <font color=\"#00cc00\">to</font> <font color=\"#ffff00\">$recipient@$interface</font> <font color=\"#00cc00\">at</font> (<font color=\"#00cc00\">$now</font>)</p>\n";
    echo "<textarea rows=\"6\" cols=\"60\" name=\"message\" style=\"font: 10pt Helvetica\"></textarea>\n";

    // Form buttons.
    //
    echo "<br><input type=\"submit\" value=\"Send X!\">\n";
    echo "<input type=\"reset\" value=\"Clear\">\n";

    echo "</form>\n";

}

?>
