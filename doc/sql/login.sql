#
# Logins in SQL.
#

DROP TABLE login;

CREATE TABLE login (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   stamp	TIMESTAMP,

   usernum	INT UNSIGNED NOT NULL,

   login	DATETIME NOT NULL,
   logout	DATETIME NOT NULL,

   host		VARCHAR(255) NOT NULL,

   reason	INT DEFAULT 0,

   PRIMARY KEY ( id, stamp ),
   INDEX ( stamp ),
   INDEX ( usernum )
)
