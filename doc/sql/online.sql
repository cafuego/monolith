# $Id$
# list of currently online users.

DROP TABLE online;

CREATE TABLE online (

   user_id	INT UNSIGNED NOT NULL,
   interface	ENUM( 'telnet', 'client', 'web' ) DEFAULT 'telnet',
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id ),

)
