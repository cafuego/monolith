#
# Error log in SQL.
#

# DROP TABLE error;

CREATE TABLE error (

   id		INT UNSIGNED NOT NULL AUTO_INCREMENT,
   stamp	TIMESTAMP,

   program	VARCHAR(10) NOT NULL,
   pid		INT UNSIGNED NOT NULL,
   usernum	INT UNSIGNED NOT NULL,

   entry	VARCHAR(128) NOT NULL,

   PRIMARY KEY  ( id, stamp ),
   INDEX ( stamp ),
   INDEX ( usernum )
)
