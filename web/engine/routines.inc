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
        if ($usernum== $author)
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

    errorimage( "$username was logged out" );
    errorimage( "You will be taken to our main page in 10 seconds");
    echo "<p>&nbsp;<br>&nbsp;<br></p>\n";
    echo "<div align=\"center\">\n";
    addbutton( "Log back in as $username", "/bbs/");
    echo "</div>\n";

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

    addimage( "Hello, $username. Welcome to Monolith BBS" );

    if($number && ($username != "Guest"))
        print "<p class=\"footer\">Your last login was on $date at $time CET from host '$hostname'.</p>\n";
    else
        print "<p class=\"footer\">Unable to retrieve your previous login details.</p>\n";
    print "<p align=\"center\"><img align=\"center\" src=\"/bbs/pix/monolith.jpg\" width=\"300\" height =\"300\" alt=\"[ image: Monolith BBS Logo ]\"></p>\n";

}

function list_x_messages($username,$location) {

    $user_id = get_usernum($username);

    connect_db();

    $result = MYSQL_QUERY("SELECT w.id AS id,u.username AS user,w.sender AS sender,w.i_sender AS i_sender,w.recipient AS recipient,w.message AS message,DATE_FORMAT(w.date, '%H:%i') AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.sender WHERE w.recipient=$user_id AND w.status='unread' AND w.i_recipient='web' ORDER BY w.date ASC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        if($number == 1)
            addimage( "You have 1 new eXpress message" );
        else
            addimage( "You have $number new eXpress messages" );

        print "<table border=\"0\" cellpadding=\"2\" cellspacing=\"4\" align=\"center\">\n";
        $i = 0;
        while ($i < $number) {

            $id = mysql_result($result,$i,"id");
            $user = mysql_result($result,$i,"user");
            $sender = mysql_result($result,$i,"sender");
            $i_sender = mysql_result($result,$i,"i_sender");
            $recipient = mysql_result($result,$i,"recipient");
            $message = mysql_result($result,$i,"message");
            $date = mysql_result($result,$i,"date");
            $reply = rawurlencode($user);

            $bing = MYSQL_QUERY("UPDATE webx SET status='read' WHERE id=$id");
            $bing = MYSQL_QUERY("UPDATE webx SET stamp=NOW() WHERE id=$id");

            print "<tr>\n";
            print "  <td align=\"left\" bgcolor=\"#333333\">\n";
            print "    <p align=\"left\"><font color=\"#00cc00\">eXpress Message from <a href=\"/bbs/index.phtml/x/$reply/$i_sender/$location\">$user@$i_sender</a> to <font color=\"#ffff00\">$username@web</font> at </font>(<font color=\"#00cc00\">$date</font>)\n";
            print "    <font color=\"#00cccc\"><pre>$message</pre></font>\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";
    }

    return;

    // See if messages were read and trash if yes.
    //
    $result = MYSQL_QUERY("SELECT w.id AS id,u.username AS user,w.i_recipient AS interface,DATE_FORMAT(w.date, '%H:%i') AS date FROM webx AS w LEFT JOIN user AS u ON u.id=w.recipient WHERE w.sender=$user_id AND w.status='read' AND w.i_sender='web' ORDER BY w.date ASC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        print "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\">\n";
        $i = 0;
        while ($i < $number) {

            $id = mysql_result($result,$i,"id");
            $user = mysql_result($result,$i,"user");
            $interface = mysql_result($result,$i,"interface");
            $date = mysql_result($result,$i,"date");

            print "<tr>\n";
            print "  <td align=\"left\">\n";
            print "    <p align=\"left\"><li><font color=\"#00cc00\">eXpress Message to <a href=\"/bbs/index.phtml/x/$user/$interface/$location\">$user@$interface</a> was read at $date CET</font>.\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";

    }

}

function x_log($username,$location) {

    $user_id = get_usernum($username);

    connect_db();

    $result = MYSQL_QUERY("SELECT id,sender,i_sender,recipient,i_recipient,message,DATE_FORMAT(date, '%H:%i') AS date,DATE_FORMAT(stamp, '%H:%i') AS stamp FROM webx AS w WHERE ((recipient=$user_id AND i_recipient='web') OR (sender=$user_id AND i_sender='web')) AND status='read' ORDER BY date DESC");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        if($number == 1) {
            addimage( "You have 1 eXpress message in your log" );
            $text = rawurlencode("(This will be removed when you log out)");
        } else {
            addimage( "You have $number eXpress messages in your log" );
            $text = rawurlencode("(These will be removed when you log out)");
        }
        echo "<div align=\"center\">\n";
        echo "<img src=\"/bbs/image.phtml/Tahoma_B,12,white/$text/\" alt=\"[ image: ". rawurldecode($text) ." ]\">\n";
        echo "</div>\n";

        print "<table border=\"0\" cellpadding=\"2\" cellspacing=\"4\" align=\"center\">\n";
        $i = 0;
        while ($i < $number) {

            $id = mysql_result($result,$i,"id");
            $sender = mysql_result($result,$i,"sender");
            $sender = get_username($sender);
            $i_sender = mysql_result($result,$i,"i_sender");
            $recipient = mysql_result($result,$i,"recipient");
            $recipient = get_username($recipient);
            $i_recipient = mysql_result($result,$i,"i_recipient");
            $message = mysql_result($result,$i,"message");
            $date = mysql_result($result,$i,"date");
            $stamp = mysql_result($result,$i,"stamp");
            $reply = rawurlencode($sender);

            print "<tr>\n";
            print "  <td align=\"left\" bgcolor=\"#333333\">\n";
            print "    <p align=\"left\"><font color=\"#00cc00\">eXpress Message from <a href=\"/bbs/index.phtml/x/$reply/$i_sender/$location\">$sender@$i_sender</a> to <font color=\"#ffff00\">$recipient@$i_recipient</font> at </font>(<font color=\"#00cc00\">$date</font>) <font color=\"#00cc00\">(read at $stamp)</font>\n";
            print "    <font color=\"#00cccc\"><pre>$message</pre></font>\n";
            print "  </td>\n";
            print "</tr>\n";

            $i++;
        }
        print "</table>\n";
    } else {
        errorimage( "There are no old Xes in your log" );
    }
}

function colorize($doing) {

    // Make escape chars usable.
    //
    $doing = rawurlencode($doing);

    // Remove misc codes.
    //
    $doing = ereg_replace("%01a", "", $doing);	// Reset.
    $doing = ereg_replace("%01f", "", $doing);	// Bold.
    $doing = ereg_replace("%01d", "", $doing);	// Dark.
    $doing = ereg_replace("%01D", "", $doing);	// Dark bg.
    $doing = ereg_replace("%01n", "", $doing);	// Start user.
    $doing = ereg_replace("%01N", "", $doing);	// End user.

    // Remove bg colours.
    //
    $doing = ereg_replace("%01W", "", $doing);	// White.
    $doing = ereg_replace("%01Y", "", $doing);	// Yellow.
    $doing = ereg_replace("%01G", "", $doing);	// Green.
    $doing = ereg_replace("%01C", "", $doing);	// Cyan.
    $doing = ereg_replace("%01B", "", $doing);	// Blue.
    $doing = ereg_replace("%01P", "", $doing);	// Purple.
    $doing = ereg_replace("%01R", "", $doing);	// Red.

    // Fix fg colours.
    //
    $doing = ereg_replace("%01w", "</font><font color=\"#ffffff\">", $doing);	// White.
    $doing = ereg_replace("%01y", "</font><font color=\"#ffff00\">", $doing);	// Yellow.
    $doing = ereg_replace("%01g", "</font><font color=\"#00ff00\">", $doing);	// Green.
    $doing = ereg_replace("%01c", "</font><font color=\"#00ffff\">", $doing);	// Cyan.
    $doing = ereg_replace("%01b", "</font><font color=\"#0000ff\">", $doing);	// Blue.
    $doing = ereg_replace("%01p", "</font><font color=\"#ff00ff\">", $doing);	// Purple.
    $doing = ereg_replace("%01r", "</font><font color=\"#ff0000\">", $doing);	// Red.

    return rawurldecode($doing);
}

function addimage( $text ) {

    // To change all images in one go.
    // Image parser only knows basics tho!
    //
    $colour = "white";
    $text = rawurlencode($text);

    echo "<div align=\"center\">\n";
    echo "<img align=\"center\" src=\"/bbs/image.phtml/Tahoma_B,18,$colour/$text/\" alt=\"[ image: ". rawurldecode($text) ." ]\">\n";
    echo "</div>\n";

}

function addbutton( $text, $url ) {

    // To change all images in one go.
    // Image parser only knows basics tho!
    //
    $colour = "yellow";
    $imgtext = rawurlencode($text);

    if($url != "")
    echo "<a href=\"$url\"><img src=\"/bbs/image.phtml/Arialblack_I,12,$colour/$imgtext/\" alt=\"[ button: $text ]\" border=\"0\"></a>\n";
    else
    echo "<img src=\"/bbs/image.phtml/Arialblack_I,12,$colour/$imgtext/\" alt=\"[ image: $text ]\">\n";

}


function errorimage( $text ) {

    $imgtext = rawurlencode($text);

    echo "<div align=\"center\">\n";
    echo "<img align=\"center\" src=\"/bbs/image.phtml/Tahoma_B,18,red/$imgtext/\" alt=\"[ image: $text ]\">\n";
    echo "</div>\n";

}

function main_menu($username, $current_url) {

    // Pick open url, so we know what to do.
    //
    $url_array=explode("/",$current_url);
    $action=$url_array[3];              // What action?
    $quad=$url_array[4];                // First arg.
    $recipient=$url_array[4];           // First arg too.
    $message=$url_array[5];             // Second arg.
    $interface=$url_array[5];           // Second arg too.
    $direction=$url_array[6];           // Third arg.
    if($action != "x")
        $location = strtr($current_url, "/", "-");
    else
        $location=$url_array[6];        // Third arg too.

    list_x_messages($username,$location);

    switch ($action) {

        case "a":	// PHP info.
            echo "<pre>\n" . phpinfo() . "</pre>\n";
            break;

        case "e":	// Enter (new).
        case "f":	// Followup (reply).
            require ('engine/post.inc');
            enter_message($username,$quad,$message,$action);
            break;

        case "l":
            require ('engine/list.inc');
            list_quads($username, 0);
            break;

        case "m":
            require ('engine/quad.inc');
            list_messages($username,$quad, 0);
            break;

        case "n":
            require ('engine/quad.inc');
            list_messages($username,$quad, 1);
            break;

        case "o":
            x_log($username,$location);
            break;

        case "q":
            do_logout($username);
            break;

        case "r":
            require ('engine/read.inc');
            read_message($username,$quad,$message,$direction);
            break;

        case "u":
            require ('engine/list.inc');
            list_quads($username, 1);
            break;

        case "w":
            require ('engine/who.inc');
            show_wholist($username,$location);
            break;

        case "x":
            require ('engine/express.inc');
            express_message($username,$recipient,$interface,$location);
            break;

        default:
            main_screen($username);
            break;

    }
}

?>
