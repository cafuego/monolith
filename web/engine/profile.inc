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

    $usernum = get_usernum($username);
    $victim_id= get_usernum($victim);

    addimage("$victim's Profile");

    $result = mysql_query("SELECT content_type FROM user WHERE id=$victim_id");
    $number = MYSQL_NUMROWS($result);
    if ($number) {
        $content_type = mysql_result($result, 0, "content_type");
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
    }

    echo "<h4 align=\"center\">( and that's all there is at the moment )</h4>\n";

}

?>
