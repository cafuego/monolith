# sql uqc (user quadcont)

# Do not drop, contains live data!
# DROP TABLE uqc;

CREATE TABLE uqc (

   id		INT UNSIGNED NOT NULL,

   cat_id	TINYINT NOT NULL,
   cat_quot	TINYINT NOT NULL DEFAULT 5,

   flags	INT UNSIGNED NOT NULL DEFAULT 0,
   mod_date	TIMESTAMP DEFAULT 0
)
