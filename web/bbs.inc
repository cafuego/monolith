<?
function connect_db() {
    $hostname = "localhost";
    $username = "root";
    $password = "";
    $db = "bbs";
    MYSQL_CONNECT($hostname, $username, $password)  OR DIE("Unable to connect to sql server.");
    @mysql_select_db("$db") or die( "Unable to select database");
}
function list_messages($username, $quad) {

    // Check user priv.
    //
    switch(check_priv($username,$quad)) {

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
    $result = MYSQL_QUERY("SELECT name FROM forum WHERE id=$quad");
    $forum = mysql_result($result,$i,"name");
    $result = MYSQL_QUERY("SELECT m.message_id AS m_id,u.username AS username,m.alias AS alias,m.subject AS subject,DATE_FORMAT(m.date,'%W, %e %M %Y') AS date,DATE_FORMAT(m.date,'%T') as time FROM message AS m LEFT JOIN user AS u ON u.id=m.author WHERE m.forum_id=$quad ORDER BY message_id,subject");
    $number = MYSQL_NUMROWS($result);
    IF($number):
        PRINT "<h3 align=\"center\"><font color=\"#FF0000\">Listing $number messages in Quadrant</font> $quad.<font color=\"#FFFF00\">$forum</font>&gt;</h3>\n<p> </p>\n";
        PRINT "<div style=\"margin-left: 150;\"><p align=\"left\">\n";
        $i = 0;
        $subcnt = 0;
        $subject = "";
        WHILE ($i < $number):
            $prevsub = $subject;
            $m_id = mysql_result($result,$i,"m_id");
            $user = mysql_result($result,$i,"username");
            $alias = mysql_result($result,$i,"alias");
            $subject = mysql_result($result,$i,"subject");
            $date = mysql_result($result,$i,"date");
            $time = mysql_result($result,$i,"time");
            $subject = htmlspecialchars($subject);
            $user = format_username($user,$alias);
            $subject = format_subject($subject);
            IF( $subject == $prevsub):
                IF( $subject != "[No subject]"):
                    $subcnt++;
                    $j = 1;
                    WHILE($j < $subcnt):
                        PRINT "&nbsp;";
                        $j++;
                    ENDWHILE;
                    PRINT "&nbsp;+&nbsp;";
                ENDIF;
            ELSE:
                $subcnt=0;
            ENDIF;
            PRINT "$m_id: &quot;<a href=\"read.phtml?quad=$quad&message=$m_id\">$subject</a>&quot; <font color=\"#00CC00\">by</font> $user <font color=\"#00CC00\">on $date</font> (<font color=\"#00CC00\">$time</font>)<br>\n";
            $i++;
        ENDWHILE;
        PRINT "</p></div>\n";
    ELSE:
        PRINT "<h3 align=\"center\">No messages found.</h3>\n";
    ENDIF;
}
function list_quads($username) {

    // Check user priv.
    //
    switch(check_priv($username,0)) {

        case -1:
            PRINT "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
            return;
        default:
            break;
    }

    $usernum = get_usernum($username);

    connect_db();

    $result = MYSQL_QUERY("SELECT ut.forum_id AS id,f.name AS name, (MAX(m.message_id)-ut.lastread) AS unread,uf.status AS status FROM usertopic as ut LEFT JOIN message as m ON (m.forum_id=ut.forum_id) LEFT JOIN forum as f ON ut.forum_id=f.id LEFT JOIN userforum AS uf ON (uf.forum_id=ut.forum_id AND uf.user_id=ut.user_id) WHERE ut.user_id=$usernum GROUP BY ut.forum_id;");
    $number = MYSQL_NUMROWS($result);
    IF($number):
        PRINT "<h3 align=\"center\">Listing Quadrants.</h3>\n";
        $i = 0;
        PRINT "<div align=\"left\">\n";
        WHILE ($i < $number):
            $id = mysql_result($result,$i,"id");
            $name = mysql_result($result,$i,"name");
            $unread = mysql_result($result, $i,"unread");
            $status = mysql_result($result,$i,"status");
            $name = htmlspecialchars($name);
           
            if($status == "invited")
                PRINT "<B>$id.<a href=\"index.phtml?quad=$id\"><font color=\"#ffff00\">$name</font></a></b> <font color=\"#ffffff\">($unread unread messages)<br>\n";

            $i++;
        ENDWHILE;
    ELSE:
        PRINT "<h3 align=\"center\">No quadrants found.</h3>\n";
    ENDIF;
}
function read_message($username, $quad, $message) {

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

    connect_db();
    IF( ($quad > -1) && ($quad < 150)):
        IF($message):
            $result = MYSQL_QUERY("SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,u.username AS username,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE forum_id=$quad AND m.message_id=$message");
            $number = MYSQL_NUMROWS($result);
            IF($number):
                $i = 0;
                WHILE ($i < $number):
                    $m_id = mysql_result($result,$i,"m_id");
                    $f_id = mysql_result($result,$i,"f_id");
                    $a_id = mysql_result($result,$i,"a_id");
                    $user = mysql_result($result,$i,"username");
                    $alias = mysql_result($result,$i,"alias");
                    $subject = mysql_result($result,$i,"subject");
                    $content = mysql_result($result,$i,"content");
                    $date = mysql_result($result,$i,"date");
                    $forum = mysql_result($result,$i,"forum");
                    $user = htmlspecialchars($user);
                    $alias = htmlspecialchars($alias);
                    $subject = htmlspecialchars($subject);
                    $content = htmlspecialchars($content);
                    $forum = htmlspecialchars($forum);
                    $user = format_username($user,$alias);
                    $subject = format_subject($subject);
                    PRINT "<h3 align=\"center\"><font color=\"#FF0000\">Listing message $message in Quadrant</font> $quad.<font color=\"#FFFF00\">$forum</font>&gt;</h3>\n<p> </p>\n";
                    PRINT "<p align=\"left\"><font color=\"#00CC00\">From</font> $user <font color=\"#00CC00\">at</font> <font color=\"#00CC00\">$date CET</font><br><font color=\"#FFFF00\">Subject</font>: <font color=\"#FFFF00\">$subject</font></p>\n";
                    PRINT "<font color=\"#00CCCC\"><pre>$content</pre></font>\n";
                    PRINT "<p align=\"left\"><font color=\"#FFFFFF\">$f_id.</font><font color=\"#00CC00\">$forum message #$m_id</font>&gt;</p>\n";
                    PRINT "<p> </p>\n";
                    $i++;
                ENDWHILE;
                $next = $message + 1;
                $previous = $message - 1;
                PRINT "<p align=\"center\"><a href=\"read.phtml?quad=$quad&message=$next\">Read Next</a> | \n";
                PRINT "<a href=\"read.phtml?quad=$quad&message=$previous\">Read Previous</a> | \n";
                PRINT "<a href=\"index.phtml?quad=$quad\">Message Index</a> | \n";
                PRINT "<a href=\"index.phtml\">Quadrant Index</a></p>\n";
            ELSE:
                PRINT "<h3 align=\"center\">Message $message not found.</h3>\n";
            ENDIF;
        ELSE:
            PRINT "<h3 align=\"center\">Don't know which message to read.</h3>\n";
        ENDIF;
    ELSE:
        PRINT "<h3 align=\"center\">Don't know which quadrant to read.</h3>\n";
    ENDIF;
}
function format_username($user, $alias) {
    IF($alias == "(null)"):
        return format_user($user);
    ELSE:
        return format_alias($alias);
    ENDIF;
}
function format_user($user) {
    IF($user == ""):
        return "<font color=\"#FF0000\">Deleted User</font>";
    ENDIF;
    IF($user == "NULL" ):
        return "<font color=\"#FF0000\">Deleted User</font>";
    ENDIF;
    return "<font color=\"#FFFF00\">$user</font>";
}
function format_alias($alias) {
    IF($alias == "" || $alias == "(null)"):
        return "<font color=\"#0000FF\">Anonymous User</font>";
    ELSE:
        return "<font color=\"#0000FF\">Anon</font> &quot;<font color=\"#0000FF\">$alias</font>&quot;";
    ENDIF;
}
function format_subject($subject) {
    IF($subject == "" || $subject == "(null)"):
        return "[No subject]";
    ENDIF;
    return $subject;
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
    else {
        PRINT "<!-- can't get usernum -->\n";
        return -1;
    }

    // Check status.
    //
    $result = MYSQL_QUERY("SELECT status FROM userforum WHERE forum_id=$forum_id AND user_id=$user_id");
    $number = MYSQL_NUMROWS($result);
    if($number) {
        $status = mysql_result($result,0,"status");
        PRINT "<!-- status: $status -->\n";
    } else {
        PRINT "<!-- can't get status -->\n";
        return 0;
    }

    if( $status == "invited" )
        return 1;
    else 
        return 0;

}
?>
