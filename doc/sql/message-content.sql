# $Id$
# Stores actual messages.
# DROP TABLE message-content;

CREATE TABLE message-content (

   # Message info.
   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,

   # Store sender and alias strings for searching purposes.
   sender	VARCHAR(20) NOT NULL,
   alias	VARCHAR(20) NOT NULL,
   subject	VARCHAR(100),
   content	BLOB NOT NULL,

   stamp	TIMESTAMP,

   PRIMARY KEY  ( id ),
   INDEX ( sender )
)
