# hosts <-> users
# M to N relationship between rooms & users

DROP TABLE userforum;

CREATE TABLE userforum (

# number of forum & user
   forum_id	INT UNSIGNED NOT NULL,
   user_id	INT UNSIGNED NOT NULL,

# is host, or not
   host		ENUM( 'y','n' ) DEFAULT 'n',

# xs status
   status	ENUM( 'invited', 'kicked', 'normal' ) DEFAULT 'normal',

# last post read by this user.
   readinfo	ENUM ('y','n') DEFAULT 'n',

# timestamp
   stamp        TIMESTAMP,

   PRIMARY KEY( forum_id, user_id )

)
