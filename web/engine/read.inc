<?

 /*
  * $id$
  */

function read_message($username, $quad, $current, $direction) {

    // Check user priv.
    //
    switch(check_priv($username,$quad)) {

        case -1:
            print "<h3 align=\"center\">You are not currently logged in. Log in first.</h3>\n";
        case 0:
            print "<h3 align=\"center\">You do not have the privileges to view this message.</h3>\n";
            return;
        default:
            break;
    }

    connect_db();

    if( ($quad > -1) && ($quad < 150)) {
        if ($current) {

            switch ($direction) {

                case "p":
                    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE forum_id=$quad AND m.message_id<$current AND m.deleted='n' ORDER BY m.message_id DESC";
                    break;

                case "n":
                    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE forum_id=$quad AND m.message_id>$current AND m.deleted='n' ORDER BY m.message_id ASC";
                    break;

                default:
                    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE forum_id=$quad AND m.message_id=$current";
                    break;

            }

            $result = MYSQL_QUERY($query_string);
            $number = MYSQL_NUMROWS($result);

            if($number) {

                // Fetch data and format.
                //
                $m_id = mysql_result($result,0,"m_id");
                $f_id = mysql_result($result,0,"f_id");
                $a_id = mysql_result($result,0,"a_id");
                $user = mysql_result($result,0,"username");
                $alias = mysql_result($result,0,"alias");
                $flag = mysql_result($result,0,"flag");
                $subject = mysql_result($result,0,"subject");
                $content = mysql_result($result,0,"content");
                $date = mysql_result($result,0,"date");
                $forum = mysql_result($result,0,"forum");
                $alias = htmlspecialchars($alias);
                $subject = htmlspecialchars($subject);
                $content = htmlspecialchars($content);
                $content = nl2br($content);
                $content = colorize($content);
                $forum = htmlspecialchars($forum);
                $user = format_username($username,$user,$alias,$flag);
                $subject = format_subject($subject);

                // Show message.
                //
                print "<h3 align=\"center\">Listing message $m_id in $forum</h3>\n<p> </p>\n";
                print "<div style=\"margin-left: 150;\">\n";
                print "<p align=\"left\"><font color=\"#00CC00\">From</font> $user <font color=\"#00CC00\">on</font> <font color=\"#00CC00\">$date CET</font><br><font color=\"#FFFF00\">Subject</font>: <font color=\"#FFFF00\">$subject</font></p>\n";
                print "<p class=\"content\"><pre><font color=\"#00CCCC\">$content</font></pre></p>\n";
                print "<p align=\"left\"><font color=\"#FFFFFF\">$f_id.</font><font color=\"#00CC00\">$forum message #$m_id</font>&gt;</p>\n";
                print "</div>\n";
                print "<p> </p>\n";

                // Don't let guests be able to post.
                //
                print "<p align=\"center\" style=\"font: 10pt helvetica;\">\n";
                if( $username != "Guest" ) {
                    print "<a href=\"/bbs/index.phtml/f/$quad/$m_id\">Reply</a> | \n";
                    print "<a href=\"/bbs/index.phtml/e/$quad/$m_id\">New message</a> | \n";
                }
                print "<a href=\"/bbs/index.phtml/r/$quad/$m_id/n\">Read Next</a> | \n";
                print "<a href=\"/bbs/index.phtml/r/$quad/$m_id/p\">Read Previous</a>\n";

                // Update lastseen!
                //
                if( $username != "Guest" ) {
                    $usernum = get_usernum($username);
                    $result = MYSQL_QUERY("UPDATE usertopic SET lastread=$current WHERE forum_id=$f_id AND user_id=$usernum");
                }

            } else {
                switch ($direction) {
                    case "p":
                        $current--;
                        break;
                    case "n":
                        $current++;
                        break;
                    default:
                        break;
                }
                print "<h3 align=\"center\">Message $current not found.</h3>\n";
            }
        } else {
            print "<h3 align=\"center\">Don't know which message to read.</h3>\n";
        }
    } else {
        print "<h3 align=\"center\">Don't know which quadrant to read.</h3>\n";
    }
}

?>
