<?

 /*
  * $id$
  */

function list_messages($username, $quad) {

    // Check user priv.
    //
    switch(check_priv($username,$quad)) {

        case -1:
            errorimage( "You are not currently logged in. Log in first!" );
            return;
        case 0:
            errorimage( "You do not have the privileges to view this quadrant!" );
            return;
        default:
            break;
    }

    connect_db();

    // Get lastseen id.
    //
    $usernum = get_usernum($username);
    $result = MYSQL_QUERY("SELECT lastread FROM usertopic WHERE forum_id=$quad AND user_id=$usernum");
    $lastseen = mysql_result($result,0,"lastread");

    // Get quad name.
    //
    $result = MYSQL_QUERY("SELECT name FROM forum WHERE id=$quad");
    $forum = mysql_result($result,0,"name");

    // List messages.
    //
    $result = MYSQL_QUERY("SELECT m.message_id AS m_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') as date FROM message AS m LEFT JOIN user AS u ON u.id=m.author WHERE m.forum_id=$quad ORDER BY message_id,subject");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        addimage( "Listing $number messages in $forum" );
        PRINT "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\" align=\"center\">\n";
        $i = 0;
        while ($i < $number) {
            $m_id = mysql_result($result,$i,"m_id");
            $user = mysql_result($result,$i,"username");
            $flag = mysql_result($result,$i,"flag");
            $alias = mysql_result($result,$i,"alias");
            $subject = mysql_result($result,$i,"subject");
            $date = mysql_result($result,$i,"date");
            $subject = htmlspecialchars($subject);
            $user = format_username($username,$user,$alias,$flag);
            $subject = format_subject($subject);

            PRINT "<tr>\n";
            PRINT "  <td align=\"left\">\n";
            if($m_id <= $lastseen)
                PRINT "    <p align=\"right\" class=\"green\">$m_id\n";
            else
                PRINT "    <p align=\"right\" class=\"green\"><font color=\"#ff0000\">New:</font> $m_id\n";
            PRINT "  </td>\n";
            PRINT "  <td align=\"left\">\n";
            PRINT "    <p align=\"left\"><a href=\"/bbs/index.phtml/r/$quad/$m_id\">$subject</a>\n";
            PRINT "    <font color=\"#00cc00\">by</font> $user\n";
            PRINT "    <font color=\"#00cc00\">on $date CET</font>\n";
            PRINT "  </td>\n";
            PRINT "</tr>\n";

            $i++;
        }
        PRINT "</table>\n";
    } else {
        errorimage( "No messages found!" );
    }
}

?>
