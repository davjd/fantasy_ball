-- standing
CREATE TABLE IF NOT EXISTS standing (
    "id" serial NOT NULL,
    user_rank integer NOT NULL,
    league_id integer NOT NULL,
    user_account_id integer NOT NULL,
    CONSTRAINT PK_standing PRIMARY KEY ("id"),
    CONSTRAINT FK_118 FOREIGN KEY (league_id) REFERENCES league ("id"),
    CONSTRAINT FK_121 FOREIGN KEY (user_account_id) REFERENCES user_account ("id")
);

CREATE INDEX fkIdx_119 ON standing (league_id);

CREATE INDEX fkIdx_122 ON standing (user_account_id);