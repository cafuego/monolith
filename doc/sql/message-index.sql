# $Id$
# New message index table.
# DROP TABLE message-index;

CREATE TABLE message-index (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
 
   sender	INT UNSIGNED NOT NULL,
   recipient	INT UNSIGNED NOT NULL,
   type		ENUM( 'mail', 'post', 'broadcast', 'x', 'chat' ),

   message_id	INT UNSIGNED NOT NULL,
   flag		ENUM( 'normal','anon','alias','forced','yell','auto','roomaide','tech','sysop','emp','admin' ),
   date		DATETIME NOT NULL,

   status	ENUM( 'read, 'unread' ),
   deleted	ENUM( 'y', 'n' ),

   # Still require reply info
   # Still require modification info

   stamp	TIMESTAMP,

   PRIMARY KEY  ( id ),
   INDEX ( sender ),
   INDEX ( recipient ),
   INDEX ( type ),
   INDEX ( status ),
   INDEX ( deleted ),
   KEY ( sender, recipient, message_id )
)
