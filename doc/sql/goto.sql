#
# Random goto's in SQL. (Makes bbs binary MUCH smaller)
#

# Contains live data, do NOT drop!!!
# DROP TABLE goto;

CREATE TABLE goto (

    id		INT NOT NULL AUTO_INCREMENT,
    goto	VARCHAR(200) NOT NULL,

    PRIMARY KEY  ( id ),
    UNIQUE ( goto )
)
