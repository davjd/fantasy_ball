#include "league_fetcher.h"

#include <fmt/core.h>
#include <pqxx/pqxx>
#include <tuple>

#include "util.h"

namespace fantasy_ball {

LeagueFetcher::LeagueFetcher(PostgreSQLFetch *psql_fetcher)
    : psql_fetcher_(psql_fetcher) {}

std::string LeagueFetcher::CreateUserAccount(const std::string &username,
                                             const std::string &email,
                                             const std::string &password,
                                             const std::string &first_name,
                                             const std::string &last_name) {
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string insert_profile_description =
      "INSERT into profile_description default values RETURNING id;";
  pqxx::row profile_row = W.exec1(insert_profile_description);
  const std::string insert_account = fmt::format(
      "INSERT into user_account(account_username, email, account_password, "
      "first_name, last_name, profile_description_id) values({}, {}, {}, {}, "
      "{}, {}) RETURNING id;",
      W.quote(username), W.quote(email), W.quote(password), W.quote(first_name),
      W.quote(last_name), profile_row[0].as<int>());
  pqxx::row account_row = W.exec1(insert_account);
  std::string generated_token = get_uuid();
  const std::string insert_auth_token = fmt::format(
      "INSERT into user_auth_token(user_account_id, token) values({}, {});",
      account_row[0].as<int>(), W.quote(generated_token));
  W.exec0(insert_auth_token);
  W.commit();
  return generated_token;
}

int LeagueFetcher::CreateLeague(std::string token,
                                const std::string &league_name) {
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return 0;
  }
  int league_settings_id = init_league_settings(user_account_id);

  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  // Create a default draft for the new league.
  const std::string insert_draft = fmt::format(
      "INSERT into draft(draft_type, draft_date, draft_time_allowed) "
      "values('standard', '2020-01-01', 60) RETURNING id;");
  pqxx::row draft_row = W.exec1(insert_draft);
  // Create the league with the default settings.
  const std::string insert_league = fmt::format(
      "INSERT into league(league_name, league_settings_id, draft_id) "
      "values({}, {}, {}) RETURNING id;",
      W.quote(league_name), league_settings_id, draft_row[0].as<int>());
  pqxx::row league_row = W.exec1(insert_league);
  W.commit();
  return league_row[0].as<int>();
}

void LeagueFetcher::JoinLeague(std::string token, int league_id) {
  if (league_id < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return;
  }
  AddLeagueMember(user_account_id, league_id);
}

void LeagueFetcher::AddLeagueMember(int user_account_id, int league_id) {
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  // TODO: Add safety checks to ensure that this insertion isn't violating the
  // league size constraint.
  const std::string insert_league =
      fmt::format("INSERT into league_membership(league_id, user_accound_id) "
                  "values({}, {});",
                  league_id, user_account_id);
  W.exec0(insert_league);
  W.commit();
}

void LeagueFetcher::MakeDraftPick(std::string token, int pick_number,
                                  int player_id_selected, int league_id,
                                  const std::string &positions_available) {
  if (league_id < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_draft_id =
      fmt::format("SELECT draft_id from league where id={};", league_id);
  pqxx::row draft_row = W.exec1(get_draft_id);
  const std::string insert_draft_pick = fmt::format(
      "INSERT into draft_selection(pick_number, player_id, draft_id, "
      "user_account_id) values({}, {}, {}, {});",
      pick_number, player_id_selected, draft_row[0].as<int>(), user_account_id);
  W.exec0(insert_draft_pick);
  const std::string insert_roster_member = fmt::format(
      "INSERT into roster_member(player_id, user_account_id, league_id, "
      "status, playable_positions) values({}, {}, {}, {}, {});",
      player_id_selected, user_account_id, league_id, W.quote("available"),
      W.quote(positions_available));
  W.exec0(insert_roster_member);
  W.commit();
}

void LeagueFetcher::UpdateLineup(
    std::string token, int lineup_slot_id,
    const std::vector<LeagueFetcher::PlayerSlot> &lineup) {
  if (lineup_slot_id < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string update_slot = "UPDATE lineup_slot SET player_id={} WHERE "
                                  "id={} AND user_account_id={};";
  for (const auto &slot : lineup) {
    const std::string &update_slot_player = fmt::format(
        update_slot, slot.player_id, lineup_slot_id, user_account_id);
    W.exec0(update_slot_player);
  }
  W.commit();
}

std::tuple<std::string, std::string, std::string, std::string>
LeagueFetcher::GetBasicUserInformation(std::string token) {
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return {};
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_basic_info =
      fmt::format("SELECT account_username, email, first_name, last_name from "
                  "user_account where id={};",
                  user_account_id);
  pqxx::row info_row = W.exec1(get_basic_info);
  W.commit();
  return std::make_tuple(
      info_row[0].as<std::string>(), info_row[1].as<std::string>(),
      info_row[2].as<std::string>(), info_row[3].as<std::string>());
}

std::tuple<std::string, std::string, int, int, int, std::string, std::string,
           int>
LeagueFetcher::GetMatchup(std::string token, int week_num, int league_id) {
  if (league_id < 1) {
    return {};
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return {};
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_matchup = fmt::format(
      "SELECT user_1_id, user_2_id, score_1, score_2, ties_count, "
      "date_start, matchup_tag, matchup_id from matchup where user_1_id={} or "
      "user_id_2={} and week_num={} and league_id={};",
      user_account_id, user_account_id, week_num, league_id);
  pqxx::row matchup_row = W.exec1(get_matchup);

  const std::string get_username_1 =
      fmt::format("SELECT account_username from user_account where id={};",
                  matchup_row[0].as<std::string>());
  pqxx::row username_1_row = W.exec1(get_username_1);
  const std::string get_username_2 =
      fmt::format("SELECT account_username from user_account where id={};",
                  matchup_row[1].as<std::string>());
  pqxx::row username_2_row = W.exec1(get_username_2);
  W.commit();
  return std::make_tuple(
      username_1_row[0].as<std::string>(), username_2_row[0].as<std::string>(),
      matchup_row[2].as<int>(), matchup_row[3].as<int>(),
      matchup_row[4].as<int>(), matchup_row[5].as<std::string>(),
      matchup_row[6].as<std::string>(), matchup_row[7].as<int>());
}

std::tuple<int, int, std::string> LeagueFetcher::GetMatch(std::string token,
                                                          int match_id) {
  if (match_id < 1) {
    return {};
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return {};
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_match = fmt::format(
      "SELECT user_id_1, user_id_2, match_date from match where match_id={};",
      match_id);
  pqxx::row match_row = W.exec1(get_match);
  W.commit();
  return std::make_tuple(match_row[0].as<int>(), match_row[1].as<int>(),
                         match_row[2].as<std::string>());
}

std::tuple<int, std::vector<LeagueFetcher::PlayerSlot>>
LeagueFetcher::GetLineup(std::string token, int match_id) {
  if (match_id < 1) {
    return {};
  }
  int user_account_id = auth_token_to_account_id(token);
  if (!user_account_id) {
    return {};
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_lineup =
      fmt::format("SELECT id, position, player_id, user_account_id from "
                  "lineup_slot where match_id={} and user_account_id={};",
                  match_id, user_account_id);
  pqxx::result lineup_result = W.exec(get_lineup);
  W.commit();

  // Go through each lineup slot and add it to the result tuple.
  int result_user_id;
  std::vector<PlayerSlot> lineup_slots;
  if (lineup_result.empty() || lineup_result[3].empty()) {
    return {};
  } else {
    result_user_id = lineup_result[0][3].as<int>();
  }
  auto result_tuple = std::make_tuple(result_user_id, lineup_slots);
  for (const auto &lineup_row : lineup_result) {
    PlayerSlot slot = {};
    slot.slot_id = lineup_row[0].as<int>();
    slot.position = lineup_row[1].as<std::string>();
    slot.player_id = lineup_row[2].as<int>();
    std::get<1>(result_tuple).push_back(slot);
  }
  return result_tuple;
}

int LeagueFetcher::init_league_settings(int commissioner_id) {
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  // Insert default waiver_settings.
  const std::string insert_waiver_settings =
      "INSERT into waiver_settings(waiver_delay, waiver_type) values(2, "
      "'standard') RETURNING id;";
  pqxx::row waiver_row = W.exec1(insert_waiver_settings);
  // Insert default transaction_settings.
  const std::string insert_transaction_settings =
      "INSERT into transaction_settings(trade_review_type, trade_review_time, "
      "max_acquisitions_per_matchup, trade_deadline, max_injury_reserves) "
      "values('standard', 2, 4, '2020-01-01', 3) RETURNING id;";
  pqxx::row transaction_row = W.exec1(insert_transaction_settings);
  // Insert league_settings using the default created waiver & transaction
  // settings.
  const std::string insert_league_settings = fmt::format(
      "INSERT into league_settings(league_type, max_users, logo_url, "
      "waiver_settings_id, transaction_settings_id, commissioner_account_id) "
      "values('head-to-head.standard', 2, '', {}, {}, {}) RETURNING id;",
      waiver_row[0].as<int>(), transaction_row[0].as<int>(), commissioner_id);
  pqxx::row settings_row = W.exec1(insert_league_settings);
  W.commit();
  return settings_row[0].as<int>();
}

int LeagueFetcher::auth_token_to_account_id(std::string token) {
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string sql_select =
      fmt::format("SELECT user_account_id from user_auth_token WHERE token={};",
                  W.quote(token));
  pqxx::row r = W.exec1(sql_select);
  W.commit();
  if (r.empty()) {
    return 0;
  }
  return r[0].as<int>();
}
} // namespace fantasy_ball