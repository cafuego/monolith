<?

 /*
  * $Id$
  */

$check = true;
$persistent_connections = true;

function connect_db() {

    $hostname = "localhost";
    $username = "root";
    $password = "";
    $db = "bbs";
    MYSQL_CONNECT($hostname, $username, $password)  OR DIE("Unable to connect to sql server.");
    @mysql_select_db("$db") or die( "Unable to select database");
}

function get_usernum($username) {

    connect_db();
    $result = MYSQL_QUERY("SELECT id FROM user WHERE username='$username'");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $user_id = mysql_result($result,0,"id");
    else
        $user_id = 0;
    return $user_id;

}

function get_username($usernum) {

    connect_db();
    $result = MYSQL_QUERY("SELECT username FROM user WHERE id=$usernum");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $username = mysql_result($result,0,"username");
    else
        $username = "";
    return $username;

}

function check_priv($username, $forum_id) {

    // Username exists?
    //
    if($username == "" || $username == NULL)
        return -1;

    if($forum_id == "" || $forum_id == NULL)
        $forum_id = 0;

    connect_db();

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

function check_friend($username, $friend) {

    // Username exists?
    //
    if($username == "" || $username == NULL)
        return -1;

    connect_db();

    // Get user number.
    //
    $result = MYSQL_QUERY("SELECT id FROM user WHERE username='$username'");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $user_id = mysql_result($result,0,"id");
    else
        return -1;

    // Get friend user number.
    //
    $result = MYSQL_QUERY("SELECT id FROM user WHERE username='$friend'");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $friend_id = mysql_result($result,0,"id");
    else
        return -2;

    // Get friend status (if online).
    //
    $result = MYSQL_QUERY("SELECT status FROM online WHERE user_id=$friend_id");
    $number = MYSQL_NUMROWS($result);
    if ($number)
        $status = mysql_result($result,0,"status");
    else
        return -3;
    if( $status == "no" )
        return -4;

    // Check status.
    //
    $result = MYSQL_QUERY("SELECT status FROM useruser WHERE user_id=$user_id AND friend_id=$friend_id");
    $number = MYSQL_NUMROWS($result);
    if($number)
        $status = mysql_result($result,0,"status");
    else
        return 1;

    if( $status == "enemy" )
        return 0;
    else 
        return 1;

}

function format_username($username,$user, $alias, $flag) {

    if( $flag == "anon" ) {
        $usernum = get_usernum($username);
        $author = get_usernum($user);
        if ($user== $author)
            return format_alias($alias,1);
        else
            return format_alias($alias,0);
    } else
        return format_user($user);
}

function format_user($user) {
    IF($user == ""):
        return "<font color=\"#ff0000\">Deleted User</font>";
    ENDIF;
    IF($user == "NULL" ):
        return "<font color=\"#ff0000\">Deleted User</font>";
    ENDIF;
    return "<font color=\"#ffff00\">$user</font>";
}

function format_alias($alias,$mine) {

    if($alias == "" || $alias == "(null)")
        if(!($mine))
            return "<font color=\"#0000ff\">Anonymous User</font>";
        else
            return "<font color=\"#0000ff\">Anonymous User</font> (<font color=\"#0000ff\">This is your post</font>)";
    else
        if(!($mine))
            return "<font color=\"#0000ff\">Anon</font> &quot;<font color=\"#0000ff\">$alias</font>&quot;";
        else
            return "<font color=\"#0000ff\">Anon</font> &quot;<font color=\"#0000ff\">$alias</font>&quot; (<font color=\"#0000ff\">This is your post</font>)";

}

function format_subject($subject) {
    IF($subject == "" || $subject == "(null)"):
        return "[No subject]";
    ENDIF;
    return $subject;
}

function do_logout($username) {

    print "<h3 align=\"center\">$username was logged out</h3>\n";
    print "<h3 align=\"center\">In 10 seconds you will be taken to Monoliths main page.</h3>\n";
    print "<h3 align=\"center\"><a href=\"/bbs/\">Log back in</a></h3>\n";

}

function main_screen($username) {

    // Get details from last login.
    //
    connect_db();

    $usernum = get_usernum($username);

    $result = MYSQL_QUERY("SELECT logout,host,DATE_FORMAT(stamp, '%W, %e %M %Y') AS date,DATE_FORMAT(stamp, '%H:%i:S') AS time FROM login WHERE user_id=$usernum ORDER BY logout DESC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        $date = mysql_result($result,0,"date");
        $time = mysql_result($result,0,"time");
        $hostname = mysql_result($result,0,"host");
    }

    print "<h2 align=\"center\">Hello, $username. Welcome to Monolith BBS.</h2>\n";
    if($number && ($username != "Guest"))
        print "<p class=\"footer\">Your last login was on $date at $time CET from host '$hostname'.</p>\n";
    else
        print "<p class=\"footer\">Unable to retrieve your previous login details.</p>\n";
    print "<p align=\"center\"><img align=\"center\" src=\"/bbs/pix/monolith.jpg\" width=\"300\" height =\"300\" alt=\"[ image: Monolith BBS Logo ]\"></p>\n";

}

function list_x_messages($username) {

    $user_id = get_usernum($username);

    connect_db();

    $result = MYSQL_QUERY("SELECT w.id AS id,u.username AS user,w.sender AS sender,w.i_sender AS i_sender,w.recipient AS recipient,w.message AS message,DATE_FORMAT(w.date, '%H:%i') AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.sender WHERE w.recipient=$user_id AND w.status='unread' AND w.i_recipient='web' ORDER BY w.date ASC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        if(number == 1)
            print "<h3 align=\"center\">You have $number eXpress message</h3>\n<p> </p>\n";
        else
            print "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\" bgcolor=\"#333333\">\n";
        $i = 0;
        while ($i < $number) {

            $id = mysql_result($result,$i,"id");
            $user = mysql_result($result,$i,"user");
            $sender = mysql_result($result,$i,"sender");
            $i_sender = mysql_result($result,$i,"i_sender");
            $recipient = mysql_result($result,$i,"recipient");
            $message = mysql_result($result,$i,"message");
            $date = mysql_result($result,$i,"date");

            $bing = MYSQL_QUERY("UPDATE webx SET status='read' WHERE id=$id");

            print "<tr>\n";
            print "  <td align=\"left\">\n";
            print "    <p align=\"left\"><font color=\"#00cc00\">eXpress Message from <a href=\"/bbs/index.phtml/x/$user/$s_interface\">$user@$s_interface</a> to <font color=\"#ffff00\">$username@web</font> at </font>(<font color=\"#00cc00\">$date</font>)\n";
            print "    <font color=\"#00cccc\"><pre>$message</pre></font>\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";
    }

    // See if messages were read and trash if yes.
    //
    $result = MYSQL_QUERY("SELECT w.id AS id,u.username AS user,DATE_FORMAT(w.date, '%H:%i') AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.recipient WHERE w.sender=$user_id AND w.status='read' AND w.i_recipient='web' ORDER BY w.date ASC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        print "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\">\n";
        $i = 0;
        while ($i < $number) {

            $id = mysql_result($result,$i,"id");
            $user = mysql_result($result,$i,"user");
            $date = mysql_result($result,$i,"date");

            $bing = MYSQL_QUERY("DELETE FROM webx WHERE id=$id");

            print "<tr>\n";
            print "  <td align=\"left\">\n";
            print "    <p align=\"left\"><li><font color=\"#00cc00\">eXpress Message to <a href=\"/bbs/index.phtml/x/$user/\">$user</a> was read at $date CET</font>.\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";

    }
}

?>
