-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** roster_member
CREATE TABLE IF NOT EXISTS roster_member (
    "id" serial NOT NULL,
    player_id int NOT NULL,
    user_account_id integer NOT NULL,
    league_id integer NOT NULL,
    status varchar(50) NOT NULL DEFAULT '',
    playable_positions varchar(50) NOT NULL,
    CONSTRAINT PK_roster_member PRIMARY KEY ("id"),
    CONSTRAINT FK_228 FOREIGN KEY (user_account_id) REFERENCES user_account ("id"),
    CONSTRAINT FK_231 FOREIGN KEY (league_id) REFERENCES league ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_229 ON roster_member (user_account_id);

CREATE INDEX IF NOT EXISTS fkIdx_232 ON roster_member (league_id);