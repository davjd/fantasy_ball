-- matchup
CREATE TABLE IF NOT EXISTS matchup (
    "id" serial NOT NULL,
    user_1_score int NOT NULL,
    user_2_score int NOT NULL,
    ties_count int NOT NULL,
    week_number int NOT NULL,
    user_1_id integer NOT NULL,
    user_2_id integer NOT NULL,
    CONSTRAINT PK_matchup PRIMARY KEY ("id"),
    CONSTRAINT FK_144 FOREIGN KEY (user_1_id) REFERENCES user_account ("id"),
    CONSTRAINT FK_147 FOREIGN KEY (user_2_id) REFERENCES user_account ("id")
);

CREATE INDEX fkIdx_145 ON matchup (user_1_id);

CREATE INDEX fkIdx_148 ON matchup (user_2_id);