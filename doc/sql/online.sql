# $Id$
# list of currently online users.

DROP TABLE online;

CREATE TABLE online (

   user_id	INT UNSIGNED NOT NULL,
   status	ENUM( 'yes', 'no' ) DEFAULT 'yes',
   interface	ENUM( 'telnet', 'client', 'web' ) DEFAULT 'telnet',
   doing	CHAR(40) NOT NULL,
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id )

)
