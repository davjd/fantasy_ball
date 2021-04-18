-- lineup_slot
CREATE TABLE IF NOT EXISTS lineup_slot (
    "id" serial NOT NULL,
    slot_position varchar(50) NOT NULL,
    player_id int NOT NULL,
    match_id integer NOT NULL,
    CONSTRAINT PK_lineup_slot PRIMARY KEY ("id"),
    CONSTRAINT FK_172 FOREIGN KEY (match_id) REFERENCES match ("id")
);

CREATE INDEX fkIdx_173 ON lineup_slot (match_id);