# $Id$
# list of currently online users.

# DO not drop, contains data.
# DROP TABLE online;

CREATE TABLE online (

   user_id	INT UNSIGNED NOT NULL,
   status	ENUM( 'yes', 'no' ) DEFAULT 'yes',
   interface	ENUM( 'telnet', 'client', 'web' ) DEFAULT 'telnet',
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id )

)
