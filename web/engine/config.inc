<?

function config_account( $username ) {

    $usernum = get_usernum($username);

    if($username == "Guest") {
        errorimage("Permission denied");
        return;
    }

    $user = new user_t;
    if( ($user->init($username)) == false ) {
        errorimage("I can't load your user data!");
        return;
    }

    addimage("Account Configuration for $username");

    echo "<p align=\"left\">You can add an image to your profile by uploading it to the BBS.\n";
    echo "    To do so, click the <b>Browse...</b> button below and locate the image you want displayed when someone profiles your account.\n";
    echo "<p align=\"left\">There are a few restrictions, though. The file <b>must</b> be in <b>GIF</b>, <b>JPEG</b> or <b>PNG</b> format.\n";
    echo "    Although you can upload other files, they will not be accepted by the BBS.\n";
    echo "    Additionally, any file larger than 20 Kilobytes will also be rejected, so don't choose a file that is too large.\n";

    echo "<p align=\"left\">\n";
    echo "<form enctype=\"multipart/form-data\" action=\"/bbs/handler.phtml\" method=\"post\">\n";
    echo "  <input type=\"hidden\" name=\"handler\" value=\"p\">\n";
    echo "  <input type=\"hidden\" name=\"username\" value=\"$username\">\n";
    echo "  Send this file: <input name=\"userfile\" type=\"file\">\n";
    echo "  <input type=\"submit\" value=\"Upload File\">\n";
    echo "</form>\n";

    echo "<p align=\"left\">This is the image that is currently in your profile:<p align=\"center\">&nbsp;\n";
    $user->add_image();

}

?>
