# $Id$

# DROP TABLE web-x;

CREATE TABLE web-x (

   id		INT UNSIGNED AUTO_INCREMENT NOT NULL,

   sender	INT UNSIGNED NOT NULL,
   recipient	INT UNSIGNED NOT NULL,
   message	BLOB NOT NULL,

   status	ENUM( 'unread', 'read' ) DEFAULT 'unread',
   stamp        TIMESTAMP,

   PRIMARY KEY	( id ),
   INDEX ( sender,recipient )

)
