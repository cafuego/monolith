# hosts <-> users
# M to N relationship between rooms & users

DROP TABLE usertopic;

CREATE TABLE usertopic (

# number of topic & user
   user_id	INT UNSIGNED NOT NULL,
   topic_id	INT UNSIGNED NOT NULL,
   forum_id	INT UNSIGNED NOT NULL,

# last post read by this user.
   lastread	INT UNSIGNED DEFAULT 0,

# timestamp
   stamp        TIMESTAMP,

   PRIMARY KEY( user_id, forum_id, topic_id )
)
