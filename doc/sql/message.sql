# quads

DROP TABLE message;

CREATE TABLE message (

   message_id	INT UNSIGNED NOT NULL,
   topic_id	INT UNSIGNED NOT NULL,
   forum_id	INT UNSIGNED NOT NULL,

   author	INT UNSIGNED,
   alias	VARCHAR(20) NOT NULL,

   subject	VARCHAR(100),
   date		DATETIME NOT NULL,

   nolines 	INT,
   content	MEDIUMTEXT,

   type		ENUM( 'anon', 'alias', 'normal' ),
   priv		ENUM( 'emp', 'sysop', 'host', 'normal' ),
   deleted	ENUM( 'y', 'n' ),
  
   stamp	TIMESTAMP,

   PRIMARY KEY  ( forum_id, topic_id, message_id ),
   INDEX ( topic_id ),
   INDEX ( forum_id )
)
