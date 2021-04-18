-- user_account
CREATE TABLE IF NOT EXISTS user_account (
    "id" serial NOT NULL,
    username varchar(50) NOT NULL,
    email varchar(100) NOT NULL,
    userpassword varchar(50) NOT NULL,
    avi_link varchar(50) NOT NULL,
    date_joined date NOT NULL,
    profile_description_id integer NOT NULL,
    CONSTRAINT PK_user_account PRIMARY KEY ("id"),
    CONSTRAINT FK_28 FOREIGN KEY (profile_description_id) REFERENCES profile_description ("id")
);

CREATE INDEX fkIdx_29 ON user_account (profile_description_id);