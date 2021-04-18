-- league_settings
CREATE TABLE IF NOT EXISTS league_settings (
    "id" serial NOT NULL,
    league_type varchar(50) NOT NULL,
    max_users integer NOT NULL,
    commissioner integer NOT NULL,
    logo_url varchar(50) NOT NULL,
    waiver_settings_id integer NOT NULL,
    transaction_settings_id integer NOT NULL,
    CONSTRAINT PK_league_settings PRIMARY KEY ("id"),
    CONSTRAINT FK_87 FOREIGN KEY (waiver_settings_id) REFERENCES waiver_settings ("id"),
    CONSTRAINT FK_90 FOREIGN KEY (transaction_settings_id) REFERENCES transaction_settings ("id")
);

CREATE INDEX fkIdx_88 ON league_settings (waiver_settings_id);

CREATE INDEX fkIdx_91 ON league_settings (transaction_settings_id);