# quads

DROP TABLE rating;

CREATE TABLE rating (

   message_id	INT UNSIGNED NOT NULL,
   topic_id	INT UNSIGNED NOT NULL,
   forum_id	INT UNSIGNED NOT NULL,

   user_id	INT UNSIGNED NOT NULL,

   score	INT DEFAULT 0,

   stamp	TIMESTAMP,

   PRIMARY KEY  ( forum_id, message_id, user_id ),
   KEY  ( topic_id, message_id,user_id ),
   INDEX ( forum_id,message_id ),
   INDEX ( topic_id,message_id ),
   INDEX ( user_id,message_id )
)
