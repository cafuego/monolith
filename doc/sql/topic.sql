# Topics
# sub-areas inside forums.

# DROP TABLE topic;

CREATE TABLE topic (

# topic name/number
   topic_id	INT UNSIGNED NOT NULL AUTO_INCREMENT,
   forum_id	INT  UNSIGNED NOT NULL,

   name		VARCHAR(40) NOT NULL,

   highest      INT UNSIGNED DEFAULT 0,

   frozen	ENUM( 'y','n' ) DEFAULT 'n',
   hidden	ENUM( 'y','n' ) DEFAULT 'n',

# timestamp
   stamp	TIMESTAMP,

   PRIMARY KEY ( topic_id ),
   UNIQUE( name )
)
