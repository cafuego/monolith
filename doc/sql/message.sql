# quads

# Do not drop, contains live data!
DROP TABLE message;

CREATE TABLE message (

   # Message info.
   message_id	INT UNSIGNED NOT NULL,
   topic_id	INT UNSIGNED NOT NULL,
   forum_id	INT UNSIGNED NOT NULL,

   # Extra display info.
   flag		ENUM( 'normal','anon','alias','forced','yell','auto','roomaide','tech','sysop','emp','admin' ),
   date		DATETIME NOT NULL,

   # Actual useful info.
   author	INT UNSIGNED NOT NULL,
   alias	VARCHAR(20) NOT NULL,
   subject	VARCHAR(100),
   content	BLOB NOT NULL,

   deleted	ENUM( 'y', 'n' ),

   # Reply info.
   r_message_id	INT UNSIGNED,
   r_forum_id	INT UNSIGNED,
   r_topic_id	INT UNSIGNED,
   r_flag	ENUM( 'normal','anon','alias','forced','yell','auto','roomaide','tech','sysop','emp','' ),
   r_date	DATETIME,
   r_author	INT UNSIGNED,
   r_alias	VARCHAR(20),
   r_subject	VARCHAR(100),

   # Modification info.
   m_message_id	INT UNSIGNED,
   m_forum_id	INT UNSIGNED,
   m_topic_id	INT UNSIGNED,
   m_date	TIMESTAMP,
   m_flag	ENUM( 'normal','anon','alias','forced','yell','auto','roomaide','tech','sysop','emp','' ),
   m_author	INT UNSIGNED,
   m_reason	ENUM( 'copied','moved','' ),

   stamp	TIMESTAMP,

   PRIMARY KEY  ( forum_id, topic_id, message_id ),
   INDEX ( topic_id ),
   INDEX ( forum_id )
)
