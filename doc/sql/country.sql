# Country info's in SQL.

DROP TABLE country;

CREATE TABLE country (

# codes: `gb' for united kingdom: (ISO 3166)
    code	CHAR(2) NOT NULL,
# codes: `gbr' for united kingdom: (ISO 3166)
    ref		CHAR(3) NOT NULL,
# codes: `826' for united kingdom: (ISO 3166)
# Saved as char coz first digit may be 0.
    num		CHAR(3) NOT NULL,
# international dial code: 31 for holland
    phone	CHAR(2),
# 'The Netherlands'
    englishname CHAR(50) NOT NULL,
# 'Nederland'
    localname	CHAR(50),

# main language 'nl' for dutch too, i think. 'en' for english
    language	CHAR(2),

    PRIMARY KEY  ( code ),
    UNIQUE ( englishname )
);

insert into country values ('NL', 'NLD', '528', '31', 'The Netherlands', 'Nederland', 'nl' )
