<?

function config( $username ) {

    echo "<form enctype=\"multipart/form-data\" action=\"/bbs/handler.phtml\" method=\"post\"\n";
    echo "  <input type=\"hidden\" name=\"handler\" value=\"p\">\n";
    echo "  <input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"10000\">\n";
    echo "  Send this file: <input name=\"userfile\" type=\"file\">\n";
    echo "  <input type=\"submit\" value=\"Upload File\">\n";
    echo "</form>\n";

}

?>

</body>
</html>
