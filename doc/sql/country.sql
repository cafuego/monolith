# Country info's in SQL.

DROP TABLE country;

CREATE TABLE country (

    code	CHAR(2) NOT NULL,
# code: `uk' for united kingdom: some ISO standard
    phone	CHAR(2),
# international dial code: 31 for holland
    englishname CHAR(50) NOT NULL,
# 'The Netherlands'
    localname	CHAR(50),
# 'Nederland'

    language	CHAR(2),
# main language 'nl' for dutch too, i think. 'en' for english

    PRIMARY KEY  ( code ),
    UNIQUE ( englishname )
);

insert into country values ('nl', '31', 'The Netherlands', 'Nederland', 'nl' )
