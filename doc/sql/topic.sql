# Topics
# sub-areas inside forums.

DROP TABLE topic;

CREATE TABLE topic (

# topic name/number
   topic_id	INT NOT NULL AUTO_INCREMENT,
   forum_id	INT NOT NULL,

   name		VARCHAR(40),

   frozen	ENUM( 'y','n' ) DEFAULT 'n',
   hidden	ENUM( 'y','n' ) DEFAULT 'n',

# timestamp
   stamp	TIMESTAMP,

   PRIMARY KEY ( topic_id, forum_id )
)
