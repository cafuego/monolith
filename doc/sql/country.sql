#
# Country info's in SQL. (Makes bbs binary MUCH smaller)
#

# DROP TABLE country;

CREATE TABLE country (

    code	CHAR(2) NOT NULL,
# code: `uk' for united kingdom: some ISO standard
    phone	NUMERIC(2,0),
# international dial code: 31 for holland
    englishname CHAR(50) NOT NULL,
# 'The Netherlands'
    localname	CHAR(50),
# 'Nederland'

    language	CHAR(2),
# main language 'nl' for dutch too, i think. 'en' for english

    PRIMARY KEY  ( code ),
    UNIQUE ( englishname )
)
