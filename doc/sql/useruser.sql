# users <-> users
# table with friends, enemies, inform logons, etc.

DROP TABLE useruser;

CREATE TABLE useruser (

   user_id	INT UNSIGNED NOT NULL,
   friend_id	INT UNSIGNED NOT NULL,

   status	SET( 'friend', 'enemy', 'inform' ),
   quickx	INT DEFAULT -1,
  
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id, friend_id )

)
