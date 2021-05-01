-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** league
CREATE TABLE IF NOT EXISTS league (
    "id" serial NOT NULL,
    league_name varchar(50) NOT NULL,
    league_settings_id integer NOT NULL,
    draft_id integer NOT NULL,
    CONSTRAINT PK_league PRIMARY KEY ("id"),
    CONSTRAINT FK_102 FOREIGN KEY (draft_id) REFERENCES draft ("id"),
    CONSTRAINT FK_93 FOREIGN KEY (league_settings_id) REFERENCES league_settings ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_103 ON league (draft_id);

CREATE INDEX IF NOT EXISTS fkIdx_94 ON league (league_settings_id);