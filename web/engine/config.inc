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

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "name" ))
        echo "<input type=\"checkbox\" name=\"name\"> Real name: $user->realname\n";
    else
        echo "<input type=\"checkbox\" name=\"name\" checked> Real name: $user->realname\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "address" ))
        echo "<input type=\"checkbox\" name=\"address\"> Address: $user->address\n";
    else
        echo "<input type=\"checkbox\" name=\"address\" checked> Address: $user->address\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "city" ))
        echo "<input type=\"checkbox\" name=\"city\"> City: $user->city\n";
    else
        echo "<input type=\"checkbox\" name=\"city\" checked> City: $user->city\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "state" ))
        echo "<input type=\"checkbox\" name=\"state\"> State: $user->state\n";
    else
        echo "<input type=\"checkbox\" name=\"state\" checked> State: $user->state\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "zip" ))
        echo "<input type=\"checkbox\" name=\"zip\"> Zip/Post code: $user->zip\n";
    else
        echo "<input type=\"checkbox\" name=\"zip\" checked> Zip/Post code: $user->zip\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "country" ))
        echo "<input type=\"checkbox\" name=\"country\"> Country: $user->country\n";
    else
        echo "<input type=\"checkbox\" name=\"country\" checked> Country: $user->country\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "phone" ))
        echo "<input type=\"checkbox\" name=\"phone\"> Phone: $user->phone\n";
    else
        echo "<input type=\"checkbox\" name=\"phone\" checked> Phone: $user->phone\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "email" ))
        echo "<input type=\"checkbox\" name=\"email\"> Email: $user->email\n";
    else
        echo "<input type=\"checkbox\" name=\"email\" checked> Email: $user->email\n";

    echo "  <p align=\"left\">\n";
    if( strstr( $user->hiddeninfo, "url" ))
        echo "<input type=\"checkbox\" name=\"url\"> Webpage: <input type=\"text\" size=\"40\" name=\"website\" value=\"$user->url\">\n";
    else
        echo "<input type=\"checkbox\" name=\"url\" checked> Webpage: <input type=\"text\" size=\"40\" name=\"website\" value=\"$user->url\">\n";

    echo "  <p align=\"center\">\n";
    echo "  <input type=\"submit\" value=\"Update\">\n";
    echo "</form>\n";

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
