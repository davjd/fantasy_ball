-- waiver_settings
CREATE TABLE IF NOT EXISTS waiver_settings (
    "id" serial NOT NULL,
    waiver_delay integer NOT NULL,
    waiver_type varchar(50) NOT NULL,
    CONSTRAINT PK_waiver_settings PRIMARY KEY ("id")
);