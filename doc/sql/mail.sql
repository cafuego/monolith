# mail

# DROP TABLE mail;

CREATE TABLE mail (

   message_id	INT UNSIGNED NOT NULL,
   sender_id	INT UNSIGNED NOT NULL,
   recipient_id	INT UNSIGNED NOT NULL,

   subject	VARCHAR(100),
   date		DATETIME NOT NULL,

   nolines 	INT,
   content	MEDIUMTEXT,

   priv		ENUM( 'emp', 'sysop', 'host', 'normal' ),
   deleted_s	ENUM( 'y', 'n' ),
   deleted_r	ENUM( 'y', 'n' ),
  
   stamp	TIMESTAMP,

   PRIMARY KEY  ( sender_id, recipient_id, message_id ),
   INDEX( sender_id ),
   INDEX( recipient_id )
)
