<?

 /*
  * $id$
  */

function enter_message($username, $quad, $current, $action) {

    // Check user priv.
    //
    switch(check_priv($username,$quad)) {

        case -1:
            PRINT "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
        case 0:
            PRINT "<h3 align=\"center\">You do not have the privileges to view this message.</h3>\n";
            return;
        default:
            break;
    }

    PRINT "<h3 align=\"center\">Sorry, but this isn't working quite yet.</h3>\n";
    PRINT "<h3 align=\"center\">Try again in a few days' time.</h3>\n";
    return;

    if( $action == "f" )
        post_reply( $username, $quad, $current );
    else
        post_new( $username, $quad );

}

function post_reply($username,$quad,$current) {

    connect_db();

    // Fetch the message we're replying to.
    //
    $result = MYSQL_QUERY("SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,DATE_FORMAT(NOW(),'%W, %e %M %Y %H:%i:%S') AS now,f.name AS forum FROM message AS m LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE forum_id=$quad AND m.message_id=$current");
    $number = MYSQL_NUMROWS($result);

    if( !($number)) {
        PRINT "<h3 align=\"center\">Error: Unable to fetch original message from database!</h3>\n";
        exit;
    }

    // Fetch data and format.
    //
    $m_id = mysql_result($result,0,"m_id");
    $f_id = mysql_result($result,0,"f_id");
    $a_id = mysql_result($result,0,"a_id");
    $user = mysql_result($result,0,"username");
    $flag = mysql_result($result,0,"flag");
    $alias = mysql_result($result,0,"alias");
    $subject = mysql_result($result,0,"subject");
    $content = mysql_result($result,0,"content");
    $date = mysql_result($result,0,"date");
    $now = mysql_result($result,0,"now");
    $forum = mysql_result($result,0,"forum");
    $user = htmlspecialchars($user);
    $alias = htmlspecialchars($alias);
    $subject = htmlspecialchars($subject);
    $content = htmlspecialchars($content);
    $content = colorize($content);
    $forum = htmlspecialchars($forum);
    $user = format_username($username,$user,$alias,$flag);
    $subject = format_subject($subject);

    // Show message.
    //
    PRINT "<h3 align=\"center\">Replying to message $m_id in $forum</h3>\n<p> </p>\n";
    PRINT "<div style=\"margin-left: 150;\">\n";
    PRINT "<p class=\"reply\"><font color=\"#00CC00\">Original from</font> $user <font color=\"#00CC00\">on</font> <font color=\"#00CC00\">$date CET</font></p>\n";
    PRINT "<pre><font color=\"#00ffff\">$content</font></pre>\n";
    PRINT "<hr size=\"1\" width=\"400\">\n<br>\n";

    // Reply form.
    //
    PRINT "<form action=\"/bbs/save.phtml\" method=\"post\">\n";

    // Hidden but required info for reply.
    //
    PRINT "<input type=\"hidden\" name=\"action\" value=\"\reply\">\n";
    PRINT "<input type=\"hidden\" name=\"autor\" value=\"\$usernum\">\n";
    PRINT "<input type=\"hidden\" name=\"forum\" value=\"\$f_id\">\n";
    PRINT "<input type=\"hidden\" name=\"r_m_id\" value=\"\$m_id\">\n";
    PRINT "<input type=\"hidden\" name=\"r_f_id\" value=\"\$f_id\">\n";
    PRINT "<input type=\"hidden\" name=\"r_a_id\" value=\"\$a_id\">\n";
    PRINT "<input type=\"hidden\" name=\"r_alias\" value=\"\$alias\">\n";

    // Show formatted header.
    //
    PRINT "<p align=\"left\"><font color=\"#00CC00\">From</font> <font color=\"#ffff00\">$username</font> <font color=\"#00CC00\">on</font> <font color=\"#00CC00\">$now CET</font><br><font color=\"#FFFF00\">Subject</font>: <font color=\"#FFFF00\">$subject</font></p>\n";
    PRINT "<textarea rows=\"20\" cols=\"70\" name=\"content\" style=\"font: 10pt Helvetica\">Enter your message here.</textarea>\n";
    PRINT "<p align=\"left\"><font color=\"#FFFFFF\">$f_id.</font><font color=\"#00CC00\">$forum New message</font>&gt;</p>\n";

    // Form buttons.
    //
    PRINT "<input type=\"submit\" value=\"Save message\">\n";
    PRINT "<input type=\"reset\" value=\"Clear\">\n";

    PRINT "</form>\n";
    PRINT "</div>\n";

}

function post_new($username,$quad) {

    connect_db();

    $result = MYSQL_QUERY("SELECT name,DATE_FORMAT(NOW(),'%W, %e %M %Y %H:%i:%S') AS now FROM forum WHERE id=$quad");
    $number = MYSQL_NUMROWS($result);

    if( !($number)) {
        PRINT "<h3 align=\"center\">Error: Unable to get quadrant details!</h3>\n";
        exit;
    }

    // Fetch data and format.
    //
    $forum = mysql_result($result,0,"name");
    $now = mysql_result($result,0,"now");

    PRINT "<h3 align=\"center\">Entering new message in $forum</h3>\n<p> </p>\n";
    PRINT "<div style=\"margin-left: 150;\">\n";

    // Reply form.
    //
    PRINT "<form action=\"/bbs/save.phtml\" method=\"post\">\n";

    // Hidden but required info for reply.
    //
    PRINT "<input type=\"hidden\" name=\"action\" value=\"\enter\">\n";
    PRINT "<input type=\"hidden\" name=\"autor\" value=\"\$usernum\">\n";
    PRINT "<input type=\"hidden\" name=\"forum\" value=\"\$quad\">\n";

    // Show formatted header.
    //
    PRINT "<p align=\"left\"><font color=\"#00CC00\">From</font> <font color=\"#ffff00\">$username</font> <font color=\"#00CC00\">on</font> <font color=\"#00CC00\">$now CET</font><br>\n";
    PRINT "<table border=\"0\"><tr><td><font color=\"#FFFF00\">Subject:</font></td><td> <input type=\"text\" size=\"30\" maxlength=\"40\" name=\"subject\" style=\"font: 10pt Helvetica\"></td</tr></table></p>\n";
    PRINT "<textarea rows=\"20\" cols=\"70\" name=\"content\" style=\"font: 10pt Helvetica\">Enter your message here.</textarea>\n";
    PRINT "<p align=\"left\"><font color=\"#FFFFFF\">$quad.</font><font color=\"#00CC00\">$forum New message</font>&gt;</p>\n";

    // Form buttons.
    //
    PRINT "<input type=\"submit\" value=\"Save message\">\n";
    PRINT "<input type=\"reset\" value=\"Clear\">\n";

    PRINT "</form>\n";
    PRINT "</div>\n";

}

?>
