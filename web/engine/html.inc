<?

 /*
  * $Id$
  */

function html_print_title($username, $location) {

    // Assign values so we can use 'em.
    $url_array=explode("/",$location);
    $action=$url_array[3];		// What action?
    $quad=$url_array[4];		// What quad?

    // Check privs and print error if the user is BAD.
    //
    switch(html_check_priv($action,$username,$quad)) {

        case -1:
            print "<title>Unknown user: Log in first.</title>\n";
            return 1;
        case 0:
            print "<title>$username@Monolith BBS: Bad, bad $username - Permission denied.</title>\n";
            return 1;
        default:
            break;

    }

    // Allowed to read, print proper title.
    //
    switch($action) {

        case "e":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Entering message in $name</title>\n";
            break;
        case "f":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Replying to message in $name</title>\n";
            break;
        case "l":
            print "<title>$username@Monolith BBS: Listing available quadrants</title>\n";
            break;
        case "m":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Listing messages in $name</title>\n";
            break;
        case "q":
            print "<title>$username@Monolith BBS: Logging out</title>\n";
            break;
        case "r":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Reading message in $name</title>\n";
            break;
        case "w":
            print "<title>$username@Monolith BBS: Looking at the Wholist [Updating every 30 seconds]</title>\n";
            print "<meta http-equiv=\"Refresh\" content=\"30; url=http://monolith.yawc.net/bbs/index.phtml/w/\">\n";
            break;
        case "x":
            $quad = rawurldecode($quad);
            print "<title>$username@Monolith BBS: Sending eXpress message to $quad</title>\n";
            break;
        default:
            print "<title>$username@Monolith BBS: Logged in</title>\n";
            break;
    }

    // Make sure we don't cache any of this.
    //
    $fmt_time = strftime("%a, %d %b %Y %H:%M:%S %Z", time());
    print "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
    print "<meta http-equiv=\"Expires\" content=\"$fmt_time\">\n";

    return ($action != "q") ? 1 : 0;

}


function html_get_usernum($username) {

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    $result = MYSQL_QUERY("SELECT id FROM user WHERE username='$username'");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $user_id = mysql_result($result,0,"id");
    else
        $user_id = 0;
    return $user_id;

}

function html_do_login($username) {

    $usernum = html_get_usernum($username);

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    $result = MYSQL_QUERY("INSERT INTO online (user_id,interface,doing) VALUES ($usernum,'web','<font color=\"#ffff00\">Surfing the web</font>')");

}


function html_do_logout($username, $hostname) {

    $usernum = html_get_usernum($username);

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    $result = MYSQL_QUERY("SELECT online AS login,(NOW()-online) AS online,NOW() AS logout FROM online WHERE user_id=$usernum AND interface='web'");
    $login = mysql_result($result, 0, "login");
    $logout = mysql_result($result, 0, "logout");
    $online = mysql_result($result, 0, "online");

    $result = MYSQL_QUERY("DELETE FROM online WHERE user_id=$usernum AND interface='web'");
    $result = MYSQL_QUERY("INSERT INTO login (user_id,login,logout,host) VALUES($user_id,FROM_UNIXTIME($login),FROM_UNIXTIME($logout),SEC_TO_TIME($online),'$hostname')");

    print "<meta http-equiv=\"Refresh\" content=\"10; url=http://monolith.yawc.net/\">\n";

}

function html_get_quadname($username,$quad) {

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    $result = MYSQL_QUERY("SELECT name FROM forum WHERE id=$quad");
    $number = MYSQL_NUMROWS($result);
    return mysql_result($result,0,"name");

}

function html_check_priv($action, $username, $forum_id) {


    if( !($action) )
        return 1;

    if ( gettype($forum_id) != integer )
        return 1;

    // Make sure the query works.
    //
    if ( !($forum_id))
        $forum_id = 0;

    // Username exists?
    //
    if($username == "" || $username == NULL)
        return -1;

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    // Get user number.
    //
    $result = MYSQL_QUERY("SELECT id FROM user WHERE username='$username'");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $user_id = mysql_result($result,0,"id");
    else
        return -1;

    // Check status.
    //
    $result = MYSQL_QUERY("SELECT status FROM userforum WHERE forum_id=$forum_id AND user_id=$user_id");
    $number = MYSQL_NUMROWS($result);
    if($number)
        $status = mysql_result($result,0,"status");
    else
        return 0;

    if( $status == "invited" )
        return 1;
    else
        return 0;

}

function html_print_foot($username, $starttime) {

    $currtime = strftime("%B %d %Y, %H:%M %Z", time());
    $endtime = microtime();

    $start = explode( " ", $starttime);
    $end = explode( " ", $endtime);

    $elapsed = ($end[0]-$start[0])+($end[1]-$start[1]);

    print "<hr size=\"1\" width=\"400\" align=\"center\">\n";
    print "<p class=\"footer\">\n";
    print "  <a href=\"/bbs/index.phtml/\">Main</a> |\n";
    print "  <a href=\"/bbs/index.phtml/w\">Wholist</a> |\n";
    print "  <a href=\"/bbs/index.phtml/l\">Quadrants</a> |\n";
    print "  <a href=\"/bbs/index.phtml/q\">Log out $username</a>\n";
    print "<hr size=\"1\" width=\"400\" align=\"center\">\n";

    print "<table border=\"0\" cellspacing=\"3\" align=\"center\">\n";
    print "<tr>\n";
    print "  <td colspan=\"2\">\n";
    print "    <p class=\"footer\">All messages posted to Monolith BBS are copyrighted to their authors, unless stated otherwise.</p>\n";
    print "  </td>\n";
    print "</tr>\n";
    print "<tr>\n";
    print "  <td>\n";
    print "    <p class=\"footer\" style=\"text-align: right;\">&copy; The Monolith Community 1999 - Generated: $currtime in ";
    printf ("%.3f", $elapsed);
    print " seconds.<br>\n    Send email to <a class=\"red\" href=\"mailto:webmaster@monolith.yawc.net\">webmaster@monolith.yawc.net</a> for suggestions and comments.</p>\n";
    print "  </td>\n";
    print "  <td>\n";
    print "    <img align=\"left\" src=\"/bbs/pix/logo.jpg\" height=\"32\" width=\"32\" alt=\"Logo\">\n";
    print "  </td>\n";
    print "</tr>\n";
    print "</table>\n";

}

?>

