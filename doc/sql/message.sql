# quads

DROP TABLE message;

CREATE TABLE message (

   # Message info.
   message_id	INT UNSIGNED NOT NULL,
   topic_id	INT UNSIGNED NOT NULL,
   forum_id	INT UNSIGNED NOT NULL,

   # Actual useful info.
   author	INT UNSIGNED NOT NULL,
   alias	VARCHAR(20),
   subject	VARCHAR(100),
   content	BLOB NOT NULL,

   # Extra display info.
   date		DATETIME NOT NULL,
   flag		ENUM( 'normal','anon','alias','forced','yell','auto','roomaide','tech','sysop','emp' ),

   # Reply info.
   r_message_id	INT UNSIGNED,
   r_forum_id	INT UNSIGNED,
   r_topic_id	INT UNSIGNED,
   r_author	INT UNSIGNED,
   r_alias	VARCHAR(20),

   # Modification info.
   m_message_id	INT UNSIGNED,
   m_forum_id	INT UNSIGNED,
   m_topic_id	INT UNSIGNED,
   m_author	INT UNSIGNED,
   m_date	TIMESTAMP,
   m_reason	ENUM( 'copied','moved','' ),

   deleted	ENUM( 'y', 'n' ),

   stamp	TIMESTAMP,

   PRIMARY KEY  ( forum_id, topic_id, message_id ),
   INDEX ( topic_id ),
   INDEX ( forum_id )
)
