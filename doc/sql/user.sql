#
# Users in SQL, should be an easy exercise.
#

# this table now contains information. --DON'T-- drop it

# DROP TABLE user;

CREATE TABLE user (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   username	VARCHAR(20) NOT NULL,
   password	VARCHAR(13),

# registration information
   name		VARCHAR(80),
   address	VARCHAR(80),
   zip          VARCHAR(10),
   city		VARCHAR(80),
   state	VARCHAR(80),
   country	VARCHAR(80),
   phone	VARCHAR(20),
   email	VARCHAR(80),

   hiddeninfo   SET( 'name','address','zip','city','state','country','phone','email','url'),

# user customizable
   doing	VARCHAR(80),
   picture	BLOB,
   profile	TEXT,

   PRIMARY KEY  ( id ),
   INDEX( username ),
   UNIQUE ( username )
)
