# sql f_quadcont

# Do not drop, contains live data!
#DROP TABLE fqc;

CREATE TABLE fqc (

   f_id		INT UNSIGNED NOT NULL,
   t_id		INT UNSIGNED NOT NULL DEFAULT 0,

   cat_id	TINYINT NOT NULL,
   cat_quot	TINYINT NOT NULL DEFAULT 0,
   cat_name	VARCHAR(24) NOT NULL,

   newbie_r	SMALLINT NOT NULL DEFAULT 5,
   flags	INT UNSIGNED NOT NULL DEFAULT 0,
   mod_time	TIMESTAMP DEFAULT 0
)
