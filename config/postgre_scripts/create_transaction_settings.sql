-- *************** SqlDBM: PostgreSQL ****************;
-- ***************************************************;
-- ************************************** transaction_settings
CREATE TABLE IF NOT EXISTS transaction_settings (
    "id" serial NOT NULL,
    trade_review_type varchar(50) NOT NULL,
    trade_review_time integer NOT NULL,
    max_acquisitions_per_matchup integer NOT NULL,
    trade_deadline date NOT NULL,
    max_injury_reserves integer NOT NULL,
    CONSTRAINT PK_transaction_settings PRIMARY KEY ("id")
);