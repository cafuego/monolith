<?php

    // URRGLY! But it *does* work.

    // Create the content-type header.
    Header("Content-type: image/png");

    // Find out for which user we do this.
    //
    $url_array = explode("?", $REQUEST_URI);
    $user_id = $url_array[1];    // What action?
    
    // Connect to the database.
    MYSQL_CONNECT("localhost", "root", "") OR DIE("");
    @mysql_select_db("bbs") or die("");

    // Fetch the encoded image data.
    //
    $result = mysql_query("select picture from user where id=$user_id");
    $data = mysql_result($result, 0, "picture");

    // Convert back to binary data and dump to output.
    //
    $data = rawurldecode($data);
    echo "$data";

?>
