<?php

function pagetitle($text, $fontfile)
{

    // Header( "Content-type: image/gif" );
    $im = imagecreate(500, 30);
    $black = ImageColorAllocate($im, 0, 0, 0);
    $white = ImageColorAllocate($im, 255, 255, 255);
    $bounds = ImageTTFBBox(20, 0, $fontfile, $text);
    $x = 500 - ((500 - ($bounds[4] - $bounds[6])) / 2);
    $y = 30 - ((30 - ($bounds[1] - $bounds[7])) / 2);
    ImageTTFText($im, 20, 0, $x, $y, $white, $fontfile, $text);
    ImageGif($im);
    ImageDestroy($im);

    return;
}

?>
