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

    // Hidden info.
    //
    echo "<p align=\"left\">Select which parts of your address information you want to show to all users.\n";
    echo "  Simply check the box for each item you want to be <b>visible</b> and click on <b>Update</b>.\n";

    echo "<form action=\"/bbs/handler.phtml\" method=\"post\">\n";
    echo "  <input type=\"hidden\" name=\"handler\" value=\"h\">\n";
    echo "  <input type=\"hidden\" name=\"username\" value=\"$username\">\n";

    echo "  <table align=\"left\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n";
    if( strstr( $user->hiddeninfo, "name" ))
        echo "<tr><td><input type=\"checkbox\" name=\"name\"> Real name:</td><td>$user->realname</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"name\" checked> Real name:</td><td>$user->realname</td></tr>\n";

    if( strstr( $user->hiddeninfo, "address" ))
        echo "<tr><td><input type=\"checkbox\" name=\"address\"> Address:</td><td>$user->address</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"address\" checked> Address:</td><td>$user->address</td></tr>\n";

    if( strstr( $user->hiddeninfo, "city" ))
        echo "<tr><td><input type=\"checkbox\" name=\"city\"> City:</td><td>$user->city</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"city\" checked> City:</td><td>$user->city</td></tr>\n";

    if( strstr( $user->hiddeninfo, "state" ))
        echo "<tr><td><input type=\"checkbox\" name=\"state\"> State:</td><td>$user->state</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"state\" checked> State:</td><td>$user->state</td></tr>\n";

    if( strstr( $user->hiddeninfo, "zip" ))
        echo "<tr><td><input type=\"checkbox\" name=\"zip\"> Zip/Post code:</td><td>$user->zip</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"zip\" checked> Zip/Post code:</td><td>$user->zip</td></tr>\n";

    if( strstr( $user->hiddeninfo, "country" ))
        echo "<tr><td><input type=\"checkbox\" name=\"country\"> Country:</td><td>$user->country</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"country\" checked> Country:</td><td>$user->country</td></tr>\n";

    if( strstr( $user->hiddeninfo, "phone" ))
        echo "<tr><td><input type=\"checkbox\" name=\"phone\"> Phone:</td><td>$user->phone</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"phone\" checked> Phone:</td><td>$user->phone</td></tr>\n";

    if( strstr( $user->hiddeninfo, "email" ))
        echo "<tr><td><input type=\"checkbox\" name=\"email\"> Email:</td><td>$user->email</td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"email\" checked> Email:</td><td>$user->email</td></tr>\n";

    if( strstr( $user->hiddeninfo, "url" ))
        echo "<tr><td><input type=\"checkbox\" name=\"url\"> Webpage:</td><td><input type=\"text\" size=\"50\" name=\"website\" value=\"$user->url\"></td></tr>\n";
    else
        echo "<tr><td><input type=\"checkbox\" name=\"url\" checked> Webpage:</td><td><input type=\"text\" size=\"50\" name=\"website\" value=\"$user->url\"></td></tr>\n";
    echo "  </table>\n";

    echo "  <p align=\"center\">\n";
    echo "  <input type=\"submit\" value=\"Update\">\n";
    echo "</form>\n";

    echo "<br clear=\"all\">\n";

    // Image.
    //
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
    $user->add_image("center");

}

?>
