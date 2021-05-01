-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** user_auth_token
CREATE TABLE IF NOT EXISTS user_auth_token (
    "id" serial NOT NULL,
    user_account_id integer NOT NULL,
    "token" varchar(50) NOT NULL,
    CONSTRAINT PK_auth_token PRIMARY KEY ("id"),
    CONSTRAINT FK_198 FOREIGN KEY (user_account_id) REFERENCES user_account ("id")
);

CREATE INDEX IF NOT EXISTS fkIdx_199 ON user_auth_token (user_account_id);