-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** profile_description
CREATE TABLE IF NOT EXISTS profile_description (
    "id" serial NOT NULL,
    level varchar(50) NOT NULL DEFAULT 'beginner',
    rating integer NOT NULL DEFAULT 0,
    win_record integer NOT NULL DEFAULT 0,
    lose_record integer NOT NULL DEFAULT 0,
    tie_record integer NOT NULL DEFAULT 0,
    CONSTRAINT PK_profile_description PRIMARY KEY ("id")
);