# $Id$
# users <-> users
# table with friends, enemies, inform logons, etc.

# Aded ICQ friends/enemies to this too. Just hope that icq numbers
# don't overflow the int here.

# DROP TABLE useruser;

CREATE TABLE useruser (

   user_id	INT UNSIGNED NOT NULL,
   friend_id	INT UNSIGNED NOT NULL,

# Distinguish between ICQ and BBS people.
   iface	ENUM( 'bbs', 'icq' ) NOT NULL,

   status	SET( 'friend', 'enemy', 'inform' ),
   quickx	INT DEFAULT -1,

# in case of ICQ, store handle too.
   handle	VARCHAR(64) NOT NULL,
  
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id, friend_id, iface ),
   INDEX( user_id ),
   INDEX( iface )

)
