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

function check_priv($username, $forum_id) {

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
    print "<h3 align=\"center\">In 5 seconds you will be taken to Monoliths main page.</h3>\n";

}

function main_screen($username) {

    print "<h3 align=\"center\">Welcome to Monolith BBS, $username</h3>\n";
    print "<p align=\"center\"><img align=\"center\" src=\"pix/monolith.jpg\" width=\"300\" height =\"300\" alt=\"[ image: Monolith BBS Logo ]\"></p>\n";

}

?>
