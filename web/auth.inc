<?php

  require('config.inc');

  function auth_verify_password( $username, $password ) {

    echo "<!-- $MYSQL_SERVER -->\n";
    echo "<!-- $MYSQL_USER -->\n";
    echo "<!-- $MYSQL_PASS -->\n";
    echo "<!-- $MYSQL_DATABASE -->\n";

    @mysql_connect($MYSQL_SERVER, $MYSQL_USER, $MYSQL_PASS) or die("Unable to connect to MySQL server");
    @mysql_select_db($MYSQL_DATABASE) or die("Unable to select BBS database");

    $query = "SELECT password FROM user WHERE username='$username'";

    $res = mysql_query($query);
    $num = mysql_numrows($res);

    if(!($num))
      return false;

    $sql_pass = mysql_result($res, 0, "password");

    if( (crypt($password, substr($sql_pass, 0, 2))) == $sql_pass)
      return true;

    return false;
  }

  function auth_head() {

    echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" \"http://www.w3.org/TR/REC-html40/loose.dtd\">\n";
    echo "<html lang=\"en\">\n";
    echo "<head>\n\n";
    echo "<meta name=\"Generator\" content=\"PHP ".phpversion()."\">\n";
    echo "<meta name=\"Process-id\" content=\"".getmypid()."\">\n";
    echo "<meta name=\"Location\" content=\"$REQUEST_URI\">\n";
    echo "<meta name=\"Rcsid\" content=\"\$Id$\">\n\n";
    echo "<meta http-equiv=\"Expires\" content=\"".gmdate("l, d F Y H:i:s")." GMT\">\n";
    echo "<meta http-equiv=\"Last-Modified\" content=\"".gmdate("l, d F Y H:i:s")." GMT\">\n";
    echo "<meta http-equiv=\"Cache-Control\" content=\"no-cache, must-revalidate\">\n";
    echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">\n";
    echo "<meta http-equiv=\"Refresh\" content=\"10; url=http://". getenv("SERVER_NAME") ."/bbs/\">\n";
    echo "<link rev=\"made\" href=\"mailto:webmaster@monolith.student.utwente.nl\">\n";
    echo "<link rel=\"StyleSheet\" href=\"/bbs/style/global.css\" type=\"text/css\">\n";
    echo "<script language=\"JavaScript\">\n";
    echo "<!--\n";
    echo "function setStatus( text ) {\n";
    echo "    window.status = text;\n";
    echo "    return true;\n";
    echo "}\n";
    echo "// -->\n";
    echo "</script>\n";
    echo "<title>$BBSNAME BBS: Could not log in</title>\n";
    echo "</head>\n";
    echo "<body bgcolor=\"#000000\" text=\"#FFFFFF\" style=\"margin-left: 0; margin-top: 0;\">\n\n";
    return;
  }

  function auth_permission_denied() {

    echo "<h2 align=\"center\">Access Denied</h2>\n";
    echo "<p align=\"center\">You need to log on before using this site.</p>\n";
    echo "<p align=\"center\">Please click <a href=\"http://". getenv("SERVER_NAME") ."/bbs/\">here</a> and log on using<br>\n";
    echo "                    your login and password.</p>\n";
    echo "<p align=\"center\">Alternatively, wait 10 seconds and you will<br>\n";
    echo "                    be taken to the login screen automatically.</p>\n";
    echo "</body>\n";
    echo "</html>\n";

    return;
  }

  function auth_invalid_userpass() {

    echo "<h2 align=\"center\">Invalid Password</h2>\n";
    echo "<p align=\"center\">The password you provided for $PHP_AUTH_USER is incorrect.</p>\n";
    echo "<p align=\"center\">Please click <a href=\"http://". getenv("SERVER_NAME") ."/bbs/\">here</a> and log on using<br>\n";
    echo "                    your login and password.</p>\n";
    echo "<p align=\"center\">Alternatively, wait 10 seconds and you will<br>\n";
    echo "                    be taken to the login screen automatically.</p>\n";
    echo "</body>\n";
    echo "</html>\n";

    return;
  }

  // Check if we have a username, check password if we do.
  //
  if(!isset($PHP_AUTH_USER)) {
    Header("WWW-Authenticate: Basic realm=\"$BBSNAME BBS\"");
    Header("HTTP/1.0 401 Unauthorized");
    auth_head();
    auth_permission_denied();
    exit;
  } else {
    if(!(auth_verify_password( $PHP_AUTH_USER, $PHP_AUTH_PW )))
      Header("WWW-Authenticate: Basic realm=\"$BBSNAME BBS\"");
      Header("HTTP/1.0 401 Unauthorized");
      auth_head();
      auth_invalid_userpass();
      exit;
    }
  }

?>

