-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** lineup_slot
CREATE TABLE IF NOT EXISTS lineup_slot (
    "id" serial NOT NULL,
    "position" varchar(50) NOT NULL,
    player_id int NOT NULL,
    match_id integer NOT NULL,
    user_account_id integer NOT NULL,
    CONSTRAINT PK_lineup_slot PRIMARY KEY ("id"),
    CONSTRAINT FK_172 FOREIGN KEY (match_id) REFERENCES match ("id"),
    CONSTRAINT FK_181 FOREIGN KEY (user_account_id) REFERENCES user_account ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_173 ON lineup_slot (match_id);

CREATE INDEX IF NOT EXISTS fkIdx_182 ON lineup_slot (user_account_id);