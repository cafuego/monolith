<?

 /*
  * $Id$
  */

function show_wholist($username, $location)
{

    // Check user priv.
    //
    switch (check_priv($username, 0)) {
        case -1 :
		errorimage("You are not currently logged in. Log in first!");
	        return;
                break;
	default:
	    break;
    }

    connect_db();

    $usernum = get_usernum($username);

    // List online users..
    //
    $result = MYSQL_QUERY("SELECT u.username AS name,o.interface AS interface,o.doing AS doing,SEC_TO_TIME(NOW()-o.date) AS online FROM online AS o LEFT JOIN user AS u ON o.user_id=u.id ORDER by online DESC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
	if ($number > 1)
	    addimage("Listing $number users online");
	else
	    addimage("Listing 1 user online");

	echo "<table border=\"0\" cellpadding=\"4\" cellspacing=\"2\" align=\"center\">\n";
	echo "<tr>\n";
	echo "  <th align=\"left\"><p align=\"left\">Username</th>\n";
	echo "  <th align=\"left\"><p align=\"left\"></th>\n";
	echo "  <th align=\"left\"><p align=\"left\">Doing</th>\n";
	echo "  <th align=\"right\"><p align=\"right\">Online for</th>\n";
	echo "  <th align=\"right\"><p align=\"right\">Via</th>\n";
	echo "</tr>\n";
	echo "<tr>\n";
	echo "  <td colspan=\"4\"><hr size=\"1\"></td>\n";
	echo "</tr>\n";

	$i = 0;
	while ($i < $number) {
	    $user = mysql_result($result, $i, "name");
	    $interface = mysql_result($result, $i, "interface");
	    $doing = mysql_result($result, $i, "doing");
	    $online = mysql_result($result, $i, "online");
	    $url_user = rawurlencode($user);
	    $url_user = strtr($url_user, " ", "+");

	    if ($interface != "web") {
		$doing = htmlspecialchars($doing);
		$doing = colorize($doing);
	    }

	    echo "<tr>\n";
	    echo "  <td align=\"left\">\n";
	    echo "    <p align=\"left\"><a href=\"/bbs/index.phtml/x/$url_user/$interface/$location\" ".mouseover("Send an eXpress message to $user").">$user</a>\n";
	    echo "  </td>\n";
	    echo "  <td align=\"left\">\n";
	    echo "    <p align=\"left\"> <a href=\"/bbs/index.phtml/p/$url_user\"".mouseover("Profile $user").">P</a> \n";
	    echo "  </td>\n";
	    echo "  <td align=\"left\">\n";
	    echo "    <p align=\"left\"><font color=\"#00cc00\">$doing</font>\n";
	    echo "  </td>\n";
	    echo "  <td align=\"right\">\n";
	    echo "    <p align=\"right\"><font color=\"#00cc00\">$online</font>\n";
	    echo "  </td>\n";
	    echo "  <td align=\"right\">\n";
	    echo "    <p align=\"right\"><font color=\"#ff00ff\">&nbsp;$interface</font>\n";
	    echo "  </td>\n";
	    echo "</tr>\n";

	    $i++;
	}
	echo "</table>\n";
    } else {
	addimage("Noone is online");
    }
    
    echo "<form action=\"/bbs/profile.phtml\" method\"get\">\n";
    echo "<p align=\"center\">Profile user: <input type=\"text\" name=\"victim\" size=\"20\" maxsize=\"21\"><input type=\"submit\" value=\"Profile\">\n";
    echo "</form>\n";
}

?>
