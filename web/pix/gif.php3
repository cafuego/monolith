<?php

    // URRGLY! But it *does* work.

    // Create the content-type header.
    // Header("Content-type: image/gif");

    // Find out for which user we do this.
    //
    $url_array = explode("?", $REQUEST_URI);
    $user_id = $url_array[1];    // What action?
    echo $url_array[0];
    echo $url_array[1];
    echo $url_array[2];
    
    // Connect to the database.
    require('../engine/routines.inc');
    connect_db();

    // Fetch the encoded image data.
    //
    $result = mysql_query("select picture from user where id=$user_id");
    $data = mysql_result($result, 0, "picture");

    // Convert back to binary data and dump to output.
    //
    $data = rawurldecode($data);
    echo "$data";

?>
