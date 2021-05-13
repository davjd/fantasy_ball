-- This script is used to create test rows for the demo and other experiments.

-- Create fake users.
INSERT INTO profile_description DEFAULT VALUES;
INSERT INTO user_account(account_username, email, account_password, profile_description_id, first_name, last_name) values('admin', 'admin@mail.com', 'password', 1, 'ad', 'min');
INSERT INTO profile_description DEFAULT VALUES;
INSERT INTO user_account(account_username, email, account_password, profile_description_id, first_name, last_name) values('bob', 'bob@mail.com', 'password', 2, 'bob', 'bobby');

-- Create fake league.
INSERT into draft(draft_type, draft_date, draft_time_allowed) values('standard', '2020-01-01', 60);
INSERT into waiver_settings(waiver_delay, waiver_type) values(2, 'standard');
INSERT into transaction_settings(trade_review_type, trade_review_time, max_acquisitions_per_matchup, trade_deadline, max_injury_reserves) values('standard', 2, 4, '2020-01-01', 3);
INSERT into league_settings(league_type, max_users, waiver_settings_id, transaction_settings_id, commissioner_account_id) values('head-to-head.standard', 2, 1, 1, 1);
INSERT into league(league_name, league_settings_id, draft_id) values('BC - League', 1, 1);

-- Add fake league member(s).
INSERT into league_membership(league_id, user_account_id, season_year) values(1, 1, '2020-2021');