#
# Users in SQL, should be an easy exercise.
#

# this table now contains information. --DON'T-- drop it

# DROP TABLE user;

CREATE TABLE user (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   username	VARCHAR(20) NOT NULL,
   password	VARCHAR(13),

# the validation key
   validation	INT,

# registration information
   name		VARCHAR(80),
   address	VARCHAR(80),
   zip          VARCHAR(10),
   city		VARCHAR(80),
   state	VARCHAR(80),
   country	VARCHAR(80),
   phone	VARCHAR(20),
   email	VARCHAR(80),
   url		VARCHAR(100),

   hiddeninfo   SET( 'name','address','zip','city','state','country','phone','email','url'),

# user customizable
   doing	VARCHAR(80),
   awaymsg	VARCHAR(100) DEFAULT "",
   xtrapflag	VARCHAR(100) DEFAULT "",
   picture	BLOB,
   profile	TEXT,

# statistics
   x_count	INT UNSIGNED DEFAULT 0,
   post_count	INT UNSIGNED DEFAULT 0,
   login_count	INT UNSIGNED DEFAULT 0,

   PRIMARY KEY  ( id ),
   INDEX( username ),
   UNIQUE ( username )
)
