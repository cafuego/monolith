# Forums

# DROP TABLE forum;

CREATE TABLE forum (

# forum name/number
   id		INT NOT NULL AUTO_INCREMENT,
   name		VARCHAR(40) NOT NULL,

# for backwards compatibility
   category_old VARCHAR(60), 
   flags	INT UNSIGNED,
   generation	CHAR,
   roominfo	CHAR,
   maxmsg	INT UNSIGNED,

# category
   category	INT,

# access structure
   readxs	SET( 'normal', 'guide', 'host', 'sysop', 'emp' ),
   writexs	SET( 'normal', 'guide', 'host', 'sysop', 'emp' ),

# forum-info
   info		TEXT,
   info_auth	INT UNSIGNED,
  
# timestamp
   stamp	TIMESTAMP,

   PRIMARY KEY ( id ),
   UNIQUE( name )

)
