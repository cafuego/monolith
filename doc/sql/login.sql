#
# Logins in SQL.
#

DROP TABLE login;

CREATE TABLE login (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   stamp	TIMESTAMP,

   user_id	INT UNSIGNED NOT NULL,

   login	DATETIME NOT NULL,
   logout	DATETIME NOT NULL,
   online	TIME NOT NULL,

   host		VARCHAR(255) NOT NULL,

   reason	INT DEFAULT 0,

   PRIMARY KEY ( id, stamp ),
   INDEX ( stamp ),
   INDEX ( user_id )
)
