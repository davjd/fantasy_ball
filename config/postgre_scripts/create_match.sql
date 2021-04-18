-- match
CREATE TABLE IF NOT EXISTS match (
    "id" serial NOT NULL,
    user_1_id integer NOT NULL,
    user_2_id integer NOT NULL,
    match_date date NOT NULL,
    matchup_id integer NOT NULL,
    CONSTRAINT PK_match PRIMARY KEY ("id"),
    CONSTRAINT FK_138 FOREIGN KEY (user_1_id) REFERENCES user_account ("id"),
    CONSTRAINT FK_141 FOREIGN KEY (user_2_id) REFERENCES user_account ("id"),
    CONSTRAINT FK_175 FOREIGN KEY (matchup_id) REFERENCES matchup ("id")
);

CREATE INDEX fkIdx_139 ON match (user_1_id);

CREATE INDEX fkIdx_142 ON match (user_2_id);

CREATE INDEX fkIdx_176 ON match (matchup_id);