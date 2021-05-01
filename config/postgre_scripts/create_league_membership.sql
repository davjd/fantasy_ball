-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** league_membership
CREATE TABLE IF NOT EXISTS league_membership (
    "id" serial NOT NULL,
    league_id integer NOT NULL,
    user_account_id integer NOT NULL,
    CONSTRAINT PK_league_membership PRIMARY KEY ("id"),
    CONSTRAINT FK_218 FOREIGN KEY (league_id) REFERENCES league ("id"),
    CONSTRAINT FK_221 FOREIGN KEY (user_account_id) REFERENCES user_account ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_219 ON league_membership (league_id);

CREATE INDEX IF NOT EXISTS fkIdx_222 ON league_membership (user_account_id);