# hosts <-> users
# M to N relationship between rooms & users

DROP TABLE usertopic;

CREATE TABLE usertopic (

# number of forum & user
   forum_id	INT UNSIGNED NOT NULL,
   user_id	INT UNSIGNED NOT NULL,
   topic_id	INT USNIGNED NOT NULL,

# last post read by this user.
   lastread	INT UNSIGNED DEFAULT 0

# timestamp
   stamp        TIMESTAMP,

   PRIMARY KEY( forum_id, user_id, topic_id )
)
