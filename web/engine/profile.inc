<?

 /*
  * $Id$
  */

function profile_user($username, $victim) {

    // Check user priv.
    //
    switch (check_priv($username, 0)) {
        case -1 :
		errorimage("You are not currently logged in. Log in first!");
	        return;
                break;
	default:
	    break;
    }

    connect_db();

    $victim = urldecode($victim);

    $usernum = get_usernum($username);
    $victim_id = get_usernum($victim);

    if($victim_id == 0) {
        errorimage("I have no idea who $victim is!");
    } else {
        addimage("$victim's Profile");

        $result = mysql_query("SELECT content_type,profile FROM user WHERE id=$victim_id");
        $number = MYSQL_NUMROWS($result);
        if ($number) {
            $content_type = mysql_result($result, 0, "content_type");
            $profile = mysql_result($result, 0, "profile");
            if (valid_image( $content_type )) {
                if( $content_type == "image/gif")
                    echo "<img align=\"center\" src=\"/bbs/pix/gif.php3?$victim_id\">\n";
                else if ($content_type == "image/jpg" || $content_type == "image/jpeg" || $content_type == "image/pjpeg")
                    echo "<img align=\"center\" src=\"/bbs/pix/jpeg.php3?$victim_id\">\n";
                else if ($content_type == "image/png")
                    echo "<img align=\"center\" src=\"/bbs/pix/png.php3?$victim_id\">\n";
                else
                    errorimage("Image is in unsupported format!");
            }
            $profile = colorize($profile);
            echo "<pre><font color=\"#00ffff\">$profile</font></pre>\n";
        } else {
            errorimage("Can't find $victim's profile!");
        }
    }
}

?>
