-- draft
CREATE TABLE IF NOT EXISTS draft (
    "id" serial NOT NULL,
    draft_type varchar(50) NOT NULL,
    draft_date date NOT NULL,
    draft_time_allowed integer NOT NULL,
    CONSTRAINT PK_draft PRIMARY KEY ("id")
);