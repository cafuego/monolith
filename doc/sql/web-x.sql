# $Id$

# DROP TABLE webx;

CREATE TABLE webx (

   id		INT UNSIGNED AUTO_INCREMENT NOT NULL,

   sender	INT UNSIGNED NOT NULL,
   recipient	INT UNSIGNED NOT NULL,
   i_sender	ENUM( 'telnet', 'client', 'web' ) DEFAULT 'telnet' NOT NULL,
   i_recipient	ENUM( 'telnet', 'client', 'web' ) DEFAULT 'telnet' NOT NULL,
   message	BLOB NOT NULL,
   date		DATETIME NOT NULL,

   status	ENUM( 'unread', 'read' ) DEFAULT 'unread' NOT NULL,
   stamp        TIMESTAMP,

   PRIMARY KEY	( id ),
   INDEX ( sender,recipient )

)
