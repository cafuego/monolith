<?

function config_account( $username ) {

    echo "<form enctype=\"multipart/form-data\" action=\"/bbs/handler.phtml\" method=\"post\">\n";
    echo "  <input type=\"hidden\" name=\"handler\" value=\"p\">\n";
    echo "  <input type=\"hidden\" name=\"recipient\" value=\"$username\">\n";
    echo "  Send this file: <input name=\"userfile\" type=\"file\">\n";
    echo "  <input type=\"submit\" value=\"Upload File\">\n";
    echo "</form>\n";

}

?>
