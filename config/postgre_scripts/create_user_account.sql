-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** user_account
CREATE TABLE IF NOT EXISTS user_account (
    "id" serial NOT NULL,
    account_username varchar(50) NOT NULL,
    email varchar(100) NOT NULL,
    account_password varchar(50) NOT NULL,
    avi_link varchar(50) NOT NULL DEFAULT '',
    date_joined date NOT NULL default '2020-01-01',
    profile_description_id integer NOT NULL,
    first_name varchar(50) NOT NULL,
    last_name varchar(50) NOT NULL,
    CONSTRAINT PK_user_account PRIMARY KEY ("id"),
    CONSTRAINT FK_28 FOREIGN KEY (profile_description_id) REFERENCES profile_description ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_29 ON user_account (profile_description_id);