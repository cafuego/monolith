<?

 /*
  * $id$
  */

function show_wholist($username) {

    // Check user priv.
    //
    switch(check_priv($username,0)) {

        case -1:
            PRINT "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
            return;
        case 0:
            PRINT "<h3 align=\"center\">You do not have the privileges to view this quadrant.</h3>\n";
            return;
        default:
            break;
    }

    connect_db();

    $usernum = get_usernum($username);

    // List online users..
    //
    $result = MYSQL_QUERY("SELECT u.username AS name,o.interface AS interface,o.doing AS doing,SEC_TO_TIME(NOW()-o.stamp) AS online FROM online AS o LEFT JOIN user AS u ON o.user_id=u.id ORDER by online DESC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        PRINT "<h3 align=\"center\">Listing $number users online</h3>\n<p> </p>\n";
        PRINT "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\">\n";
        PRINT "<tr>\n";
        PRINT "  <th align=\"left\"><p align=\"left\"><b>Username</b></th>\n";
        PRINT "  <th align=\"left\"><p align=\"left\"><b>Using</b></th>\n";
        PRINT "  <th align=\"left\"><p align=\"left\"><b>Doing</b></th>\n";
        PRINT "  <th align=\"right\"><p align=\"right\"><b></b>Online</th>\n";
        PRINT "</tr>\n";
        $i = 0;
        while ($i < $number) {
            $user = mysql_result($result,$i,"user");
            $interface = mysql_result($result,$i,"interface");
            $doing = mysql_result($result,$i,"doing");
            $online = mysql_result($result,$i,"online");

            PRINT "<tr>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\"><font color=\"#00cc00\">$user</font>\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\"><font color=\"#ff00ff\">&nbsp;$interface&nbsp;</font>\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\"><font color=\"#00cc00\">$doing</font>\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"right\">\n";
            PRINT "    <p align=\"right\"><font color=\"#00cc00\">$online</font>\n";
            PRINT "  </td>\n";
            PRINT "</tr>\n";

            $i++;
        }
        PRINT "</table>\n";
    } else {
        PRINT "<h3 align=\"center\">Noone is online.</h3>\n";
    }
}

?>
