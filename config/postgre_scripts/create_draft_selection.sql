-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** draft_selection
CREATE TABLE IF NOT EXISTS draft_selection (
    "id" serial NOT NULL,
    pick_number integer NOT NULL,
    player_id integer NOT NULL,
    draft_id integer NOT NULL,
    user_account_id integer NOT NULL,
    CONSTRAINT PK_draft_selection PRIMARY KEY ("id"),
    CONSTRAINT FK_57 FOREIGN KEY (draft_id) REFERENCES draft ("id"),
    CONSTRAINT FK_71 FOREIGN KEY (user_account_id) REFERENCES user_account ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_58 ON draft_selection (draft_id);

CREATE INDEX IF NOT EXISTS fkIdx_72 ON draft_selection (user_account_id);