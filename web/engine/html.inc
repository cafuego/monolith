<?

 /*
  * $Id$
  */

function html_print_title($username, $location) {

    // Assign values so we can use 'em.
    $url_array=explode("/",$location);
    $action=$url_array[3];		// What action?
    $quad=$url_array[4];		// What quad?
    $interface=$url_array[5];

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
        case "n":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Listing unread messages in $name</title>\n";
            break;
        case "q":
            print "<title>$username@Monolith BBS: Logging out</title>\n";
            break;
        case "r":
            $name = html_get_quadname($username,$quad);
            print "<title>$username@Monolith BBS: Reading message in $name</title>\n";
            break;
        case "u":
            print "<title>$username@Monolith BBS: Listing quadrants with unread messages</title>\n";
            break;
        case "w":
            // Stop auto-update if there are new x-es.
            //
            if( html_are_there_xs($username) ) {
                print "<title>$username@Monolith BBS: Looking at the Wholist [New X, not updating!]</title>\n";
            } else {
                print "<meta http-equiv=\"Refresh\" content=\"60; url=http://monolith.yawc.net/bbs/index.phtml/w/\">\n";
                print "<title>$username@Monolith BBS: Looking at the Wholist [Updating every minute]</title>\n";
            }
            break;
        case "x":
            $quad = rawurldecode($quad);
            print "<title>$username@Monolith BBS: Sending eXpress message to $quad@$interface</title>\n";
            break;
        default:
            print "<title>$username@Monolith BBS: Logged in</title>\n";
            break;
    }

    // Make sure we don't cache any of this.
    //

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

    $result = MYSQL_QUERY("SELECT stamp FROM online WHERE user_id=$usernum AND interface='web'");
    $number = MYSQL_NUMROWS($result);

    if(!($number))
        $result = MYSQL_QUERY("INSERT INTO online (user_id,interface,doing,date) VALUES ($usernum,'web','<font color=\"#ffff00\">Surfing the web in style!</font>',NOW())");
   
}


function html_do_logout($username, $hostname) {

    $usernum = html_get_usernum($username);

    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");

    $result = MYSQL_QUERY("SELECT stamp AS login,(NOW()-stamp) AS online,NOW() AS logout FROM online WHERE user_id=$usernum AND interface='web'");
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
    if($number)
    return mysql_result($result,0,"name");
    else
    return "";

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

function html_print_foot($username, $starttime, $location) {

    $url_array=explode("/",$location);
    $endtime = microtime();

    $start = explode( " ", $starttime);
    $end = explode( " ", $endtime);

    // Don't show menu if we're logging off.
    //
    if($url_array[3] != "q") {
        echo "<hr size=\"1\" width=\"400\" align=\"center\">\n";
        echo "<p class=\"footer\">\n";
        echo "  <a href=\"$location\">Check X Messages</a> <b>|</b>\n";
        echo "  <a href=\"/bbs/index.phtml/w\">View Wholist</a> <b>|</b>\n";
        echo "  <a href=\"/bbs/index.phtml/l\">All Quadrants</a> <b>|</b>\n";
        echo "  <a href=\"/bbs/index.phtml/u\">Unread Messages</a>\n";
        echo "<p class=\"footer\">\n";
        echo "  <a href=\"/bbs/index.phtml/\">Main Page</a> <b>|</b>\n";
        echo "  <a href=\"/bbs/index.phtml/q\">Log out $username</a>\n";
        echo "<hr size=\"1\" width=\"400\" align=\"center\">\n";
    }

    echo "<table border=\"0\" cellspacing=\"3\" align=\"center\">\n";
    echo "<tr>\n";
    echo "  <td colspan=\"2\">\n";
    echo "    <p class=\"footer\">All messages posted to Monolith BBS are copyrighted to their authors, unless stated otherwise.</p>\n";
    echo "  </td>\n";
    echo "</tr>\n";
    echo "<tr>\n";
    echo "  <td>\n";
    echo "    <p class=\"footer\" style=\"text-align: right;\">&copy; The Monolith Community 1999 -\n";
    echo "      Generated: ". gmdate("l, d F Y H:i:s", time()) ." GMT.<br>\n";
    echo "      Send email to <a class=\"red\" href=\"mailto:webmaster@monolith.yawc.net\">webmaster@monolith.yawc.net</a> for suggestions and comments.</p>\n";
    echo "  </td>\n";
    echo "  <td>\n";
    echo "    <img align=\"left\" src=\"/bbs/pix/logo.jpg\" height=\"32\" width=\"32\" alt=\"Logo\">\n";
    echo "  </td>\n";
    echo "</tr>\n";
    echo "</table>\n";

    $elapsed = ($end[0]-$start[0])+($end[1]-$start[1]);
    printf( "<!-- Gen: %.4f sec. -->\n", $elapsed );

}

function html_are_there_xs($username) {

    $user_id = html_get_usernum($username);
    MYSQL_CONNECT("localhost", "root", "")  OR DIE("Unable to connect to SQL server");
    @mysql_select_db("bbs") or die( "Unable to select database");
    $result = MYSQL_QUERY("SELECT w.id AS id,u.username AS user,w.sender AS sender,w.i_sender AS i_sender,w.recipient AS recipient,w.message AS message,DATE_FORMAT(w.date, '%H:%i') AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.sender WHERE w.recipient=$user_id AND w.status='unread' AND w.i_recipient='web' ORDER BY w.date ASC");
    return MYSQL_NUMROWS($result);

}

?>

