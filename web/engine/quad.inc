<?

 /*
  * $id$
  */

function list_messages($username, $quad, $listunread) {

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
    if($listunread)
        $result = MYSQL_QUERY("SELECT m.message_id AS m_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') as date FROM message AS m LEFT JOIN user AS u ON u.id=m.author WHERE m.forum_id=$quad AND m.message_id>=$lastseen ORDER BY message_id,subject");
    else
        $result = MYSQL_QUERY("SELECT m.message_id AS m_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') as date FROM message AS m LEFT JOIN user AS u ON u.id=m.author WHERE m.forum_id=$quad ORDER BY message_id,subject");

    $number = MYSQL_NUMROWS($result);
    if ($number) {
        if($listunread)
            if($number != 1)
                addimage( "Listing $number unread messages in $forum" );
            else
                errorimage( "No unread messages, listing 1 old message!" );
        else
            if($number != 1)
                addimage( "Listing $number messages in $forum" );
            else
                addimage( "Listing 1 message in $forum" );

        echo "<table border=\"0\" cellpadding=\"0\" cellspacing=\"4\" align=\"center\">\n";
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

            echo "<tr>\n";
            echo "  <td align=\"right\" valign=\"top\">\n";
            if($m_id <= $lastseen)
                echo "    <p align=\"right\" class=\"green\">$m_id\n";
            else
                echo "    <p align=\"right\" class=\"green\"><font color=\"#ff0000\">New:</font> $m_id\n";
            echo "  </td>\n";
            echo "  <td align=\"left\" valign=\"top\">\n";
            echo "    <p align=\"left\"><font color=\"#00cc00\">Posted on $date CET</font><br>\n";
            echo "     <a href=\"/bbs/index.phtml/r/$quad/$m_id\">$subject</a> <font color=\"#00cc00\">by</font> $user\n";
            echo "  </td>\n";
            echo "</tr>\n";

            $i++;
        }
        echo "</table>\n";
    } else {
        if($listunread)
            errorimage( "No unread messages found!" );
        else
            errorimage( "No messages found!" );
    }
}

?>
