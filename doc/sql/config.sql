#
# BBS COnfigs in SQL table.
#

# Contains data, do NOT drop!!!
# DROP TABLE config;

CREATE TABLE config (

    id		INT NOT NULL AUTO_INCREMENT,
    name	VARCHAR(30) NOT NULL,

/* rooms */
    forum	VARCHAR(20) NOT NULL,
    forum_pl	VARCHAR(22) NOT NULL,

/* posts */
    message	VARCHAR(12) NOT NULL,
    message_pl	VARCHAR(14) NOT NULL,
  
/* x-es */
    x		VARCHAR(12) NOT NULL,
    x_mes	VARCHAR(12) NOT NULL,
    x_mes_pl	VARCHAR(14) NOT NULL,

/* users */
    user	VARCHAR(12) NOT NULL,
    user_pl	VARCHAR(14) NOT NULL,
    username	VARCHAR(12) NOT NULL,
  
/* wholist */
    doing	VARCHAR(20) NOT NULL,
    location	VARCHAR(16) NOT NULL,
    idle	VARCHAR(18) NOT NULL,

/* channels */
    chat	VARCHAR(20) NOT NULL,
    channel	VARCHAR(20) NOT NULL,
   
/* titles */
    admin	VARCHAR(20) NOT NULL,
    wizard	VARCHAR(20) NOT NULL,
    sysop	VARCHAR(20) NOT NULL,
    programmer	VARCHAR(20) NOT NULL,
    roomaide	VARCHAR(20) NOT NULL,
    guide	VARCHAR(20) NOT NULL,

    INDEX( id, name ),
    PRIMARY KEY  ( id ),
    UNIQUE ( name )
)
