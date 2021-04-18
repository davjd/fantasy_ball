-- profile_description
CREATE TABLE IF NOT EXISTS profile_description (
    "id" serial NOT NULL,
    user_level varchar(50) NOT NULL,
    rating integer NOT NULL,
    win_record integer NOT NULL,
    lose_record integer NOT NULL,
    tie_record integer NOT NULL,
    CONSTRAINT PK_profile_description PRIMARY KEY ("id")
);