# global x log.

# Do not drop, contains live data!
# DROP TABLE express;

CREATE TABLE express (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   user_id	INT UNSIGNED NOT NULL,

   date		DATETIME NOT NULL,
   sender	VARCHAR(60),
   recipient	VARCHAR(60),
   message	BLOB,

   sender_priv	INT UNSIGNED NOT NULL,
   override	INT NOT NULL,
   ack		INT NOT NULL,

   stamp	TIMESTAMP,

   PRIMARY KEY	( id ),
   INDEX	( user_id ),
   INDEX	( date )
)
