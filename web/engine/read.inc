<?

 /*
  * $Id$
  */

function read_message($username, $quad, $current, $direction)
{

    // Check user priv.
    //
    switch (check_priv($username, $quad)) {

	    case -1 :
		errorsound();
	    errorimage("You are not currently logged in. Log in first!");
	    return;
	case 0:
	    sorrysound();
	    errorimage("You do not have the privileges to view this message");
	    return;
	default:
	    break;
    }

    connect_db();


    if (($quad > -1) && ($quad < 150)) {

	$usernum = get_usernum($username);

	if ($current) {

	    switch ($direction) {

		case "p":
		    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,t.highest AS highest,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN topics AS t ON t.forum_id=m.forum_id LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE m.forum_id=$quad AND m.message_id<$current AND m.deleted='n' ORDER BY m.message_id DESC";
		    break;

		case "n":
		    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,t.highest AS highest,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN topics AS t ON t.forum_id=m.forum_id LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE m.forum_id=$quad AND m.message_id>$current AND m.deleted='n' ORDER BY m.message_id ASC";
		    break;

		default:
		    $query_string = "SELECT m.message_id AS m_id,m.forum_id AS f_id,m.topic_id,m.author AS a_id,t.highest AS highest,u.username AS username,m.flag AS flag,m.alias AS alias,m.subject AS subject,m.content AS content,DATE_FORMAT(m.date,'%W, %e %M %Y %H:%i:%S') AS date,f.name AS forum FROM message AS m LEFT JOIN topics AS t ON t.forum_id=m.forum_id LEFT JOIN user AS u ON u.id=m.author LEFT JOIN forum AS f ON f.id=m.forum_id WHERE m.forum_id=$quad AND m.message_id=$current";
		    break;

	    }

	    $result = MYSQL_QUERY($query_string);
	    $number = MYSQL_NUMROWS($result);

	    if ($number) {

		// Fetch data and format.
		//
		$m_id = mysql_result($result, 0, "m_id");
		$f_id = mysql_result($result, 0, "f_id");
		$a_id = mysql_result($result, 0, "a_id");
		$highest = mysql_result($result, 0, "highest");
		$user = mysql_result($result, 0, "username");
		$alias = mysql_result($result, 0, "alias");
		$flag = mysql_result($result, 0, "flag");
		$subject = mysql_result($result, 0, "subject");
		$content = mysql_result($result, 0, "content");
		$date = mysql_result($result, 0, "date");
		$forum = mysql_result($result, 0, "forum");
		$alias = htmlspecialchars($alias);
		$subject = htmlspecialchars($subject);
		$content = htmlspecialchars($content);
                if( $username == "Cafuego" )
		    $content = mes_link_urls($content);
		$content = colorize($content);
		$user = format_username($username, $user, $alias, $flag);
		$subject = format_subject($subject);

		addimage("Reading message $m_id in $forum");

		// Convert *after* generating image.
		//
		$forum = htmlspecialchars($forum);

		// Show message.
		//
		echo "<p align=\"left\"><font color=\"#00CC00\">From</font> $user";
		if ($flag != "anon")
		    echo " <font color=\"#00CC00\">on</font> <font color=\"#00CC00\">$date CET</font><br>\n";
		else
		    echo "<br>\n";
		echo "<font color=\"#FFFF00\">Subject</font>: <font color=\"#FFFF00\">$subject</font></p>\n";
		echo "<pre><font color=\"#00ffff\">$content</font></pre>\n";
		echo "<p align=\"left\"><font color=\"#FFFFFF\">$f_id.</font><font color=\"#00CC00\">$forum</font> (<font color=\"#00CC00\">message $m_id of $highest</font>)&gt;</p>\n";
		echo "<p>&nbsp;<br></p>\n";

		// Update lastseen!
		//
		if ($username != "Guest") {
		    $result = MYSQL_QUERY("SELECT lastread FROM usertopic WHERE forum_id=$f_id AND user_id=$usernum");
		    $lastread = mysql_result($result, 0, "lastread");
		    if ($lastread < $m_id)
			$result = MYSQL_QUERY("UPDATE usertopic SET lastread=$m_id WHERE forum_id=$f_id AND user_id=$usernum");
		}
		echo "<div align=\"center\">\n";
		if ($m_id != $highest) {
		    addbutton("Read Next", "/bbs/index.phtml/r/$quad/$m_id/n", "Read the next message in $forum");
		} else {
		    // Jump to next quad eh...
		    //
		    $result = MYSQL_QUERY("SELECT ut.forum_id AS forum_id,f.name AS forum_name,(t.highest-ut.lastread) AS forum_unread,uf.status AS status FROM usertopic as ut LEFT JOIN topics AS t ON ut.forum_id=t.forum_id LEFT JOIN message as m ON (m.forum_id=ut.forum_id) LEFT JOIN forum as f ON ut.forum_id=f.id LEFT JOIN userforum AS uf ON (uf.forum_id=ut.forum_id AND uf.user_id=ut.user_id) WHERE ut.user_id=$usernum AND uf.status='invited' AND ut.lastread < t.highest GROUP BY ut.forum_id ORDER BY ut.forum_id ASC;");
		    $number = MYSQL_NUMROWS($result);
		    if ($number) {
			$forum_id = mysql_result($result, 0, "forum_id");
			$forum_name = mysql_result($result, 0, "forum_name");
			$forum_unread = mysql_result($result, 0, "forum_unread");
			if ($number != 1) {
			    if ($forum_unread != 1)
				addbutton("Next quadrant with unread messages", "/bbs/index.phtml/n/$forum_id", "Read next quadrant with unread messages: $forum_name, $forum_unread unread messages ($number forums left)");
			    else
				addbutton("Next quadrant with unread messages", "/bbs/index.phtml/n/$forum_id", "Read next quadrant with unread messages: $forum_name, 1 unread message ($number forums left)");
			} else {
			    if ($forum_unread != 1) {
				addbutton("Next quadrant with unread messages", "/bbs/index.phtml/n/$forum_id", "Read next quadrant with unread messages: $forum_name, unread $forum_unread messages (last forum)");
			    } else {
				addbutton("Next quadrant with unread messages", "/bbs/index.phtml/n/$forum_id", "Read next quadrant with unread messages: $forum_name, unread 1 message (last forum)");
			    }
			}
		    } else {
			addbutton("No unread messages left on the BBS", "", "");
		    }
		}
		addbutton("Read Previous", "/bbs/index.phtml/r/$quad/$m_id/p", "Read the previous message in $forum");
		// Don't let guests be able to post.
		//
		// if( $username != "Guest" ) {
		//    addbutton("        ", "", "");
		//    addbutton("Reply to this message", "/bbs/index.phtml/f/$quad/$m_id", "Reply to this message");
		//    addbutton("Post new message", "/bbs/index.phtml/e/$quad/$m_id", "Post a new message in $forum");
		//}
		echo "</div>\n";
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
		errorsound();
		errorimage("Message $current not found!");
	    }
	} else {
	    errorsound();
	    errorimage("Don't know which message to read!");
	}
    } else {
	errorsound();
	errorimage("Don't know which quadrant to read!");
    }
}

/*
 * Looks for URLs in $text and turns them into hyperlinks.
 */
function mes_link_urls( $text ) {

    eregi("http://.* ", $text, $http_list);
    if(isset($http_list)) {
        $url = current($http_list);
        $tmplist = split(" ", $url);
        $url = $tmplist[0];
        do {
            echo "<a href=\"$url\">$url</a>\n";
        } while($url = next($http_list));
    } else {
       echo "<p>No http links found.\n";
    }

    eregi("ftp://.* ", $text, $ftp_list);
    if(isset($ftp_list)) {
        $url = current($ftp_list);
        $tmplist = split(" ", $url);
        $url = $tmplist[0];
        do {
            echo "<a href=\"$url\">$url</a>\n";
        } while($url = next($ftp_list));
    } else {
       echo "<p>No ftp links found.\n";
    }

    return $text;

    return $text;
}

?>
