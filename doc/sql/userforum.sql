# hosts <-> users
# M to N relationship between rooms & users

# DROP TABLE userforum;

CREATE TABLE userforum (

# number of forum & user
   forum_id	INT UNSIGNED NOT NULL,
   user_id	INT UNSIGNED NOT NULL,

# is host, or not
   host		ENUM( 'y','n' ) NOT NULL DEFAULT 'n',

# xs status
   status	ENUM( 'invited', 'kicked', 'normal' ) DEFAULT 'normal',

# last post read by this user.
   readinfo	ENUM ('y','n') DEFAULT 'n',

# last seen post in forum by user.
   lastseen	INT UNSIGNED DEFAULT 0,

# timestamp
   stamp        TIMESTAMP,

   PRIMARY KEY( forum_id, user_id ),
   INDEX( forum_id, user_id, host )

)
