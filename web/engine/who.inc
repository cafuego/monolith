<?

 /*
  * $id$
  */

function show_wholist($username,$location) {

    // Check user priv.
    //
    switch(check_priv($username,0)) {

        case -1:
            print "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
            return;
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
        print "<h3 align=\"center\">Listing $number users online</h3>\n<p> </p>\n";
        print "<table border=\"0\" cellpadding=\"4\" cellspacing=\"2\" align=\"center\">\n";
        print "<tr>\n";
        print "  <th align=\"left\"><p align=\"left\">Username</th>\n";
        print "  <th align=\"left\"><p align=\"left\">Doing</th>\n";
        print "  <th align=\"right\"><p align=\"right\">Online for</th>\n";
        print "  <th align=\"right\"><p align=\"right\">Via</th>\n";
        print "</tr>\n";
        print "<tr>\n";
        print "  <td colspan=\"4\"><hr size=\"1\"></td>\n";
        print "</tr>\n";
        $i = 0;
        while ($i < $number) {
            $user = mysql_result($result,$i,"name");
            $interface = mysql_result($result,$i,"interface");
            $doing = mysql_result($result,$i,"doing");
            $online = mysql_result($result,$i,"online");
            $url_user = rawurlencode($user);
            $url_user = strtr($url_user, " ", "+");

            if($doing == "(null)")
                $doing == "";

            print "<tr>\n";
            print "  <td align=\"left\">\n";
            print "    <p align=\"left\"><a href=\"/bbs/index.phtml/x/$url_user/$interface/$location\">$user</a>\n";
            print "  </td>\n";
            print "  <td align=\"left\">\n";
            print "    <p align=\"left\"><font color=\"#00cc00\">$doing</font>\n";
            print "  </td>\n";
            print "  <td align=\"right\">\n";
            print "    <p align=\"right\"><font color=\"#00cc00\">$online</font>\n";
            print "  </td>\n";
            print "  <td align=\"right\">\n";
            print "    <p align=\"right\"><font color=\"#ff00ff\">&nbsp;$interface&nbsp;</font>\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";
    } else {
        print "<h3 align=\"center\">Noone is online.</h3>\n";
    }
}

?>
