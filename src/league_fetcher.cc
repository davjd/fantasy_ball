#include "league_fetcher.h"

#include <fmt/core.h>
#include <pqxx/pqxx>
#include <tuple>

#include "util.h"

namespace fantasy_ball {

LeagueFetcher::LeagueFetcher(PostgreSQLFetch *psql_fetcher)
    : psql_fetcher_(psql_fetcher) {}

void LeagueFetcher::CreateUserAccount(
    const leagueservice::CreateUserAccountRequest *request,
    leagueservice::AuthToken *reply) {
  try {
    auto *connection = psql_fetcher_->GetCurrentConnection();
    pqxx::work W{*connection};
    const std::string insert_profile_description =
        "INSERT into profile_description default values RETURNING id;";
    pqxx::row profile_row = W.exec1(insert_profile_description);
    const std::string insert_account = fmt::format(
        "INSERT into user_account(account_username, email, account_password, "
        "first_name, last_name, profile_description_id) values({}, {}, {}, {}, "
        "{}, {}) RETURNING id;",
        W.quote(request->username()), W.quote(request->email()),
        W.quote(request->password()), W.quote(request->first_name()),
        W.quote(request->last_name()), profile_row[0].as<int>());
    pqxx::row account_row = W.exec1(insert_account);
    std::string generated_token = get_uuid();
    const std::string insert_auth_token = fmt::format(
        "INSERT into user_auth_token(user_account_id, token) values({}, {});",
        account_row[0].as<int>(), W.quote(generated_token));
    W.exec0(insert_auth_token);
    W.commit();
    reply->set_token(generated_token);
  } catch (std::exception const &e) {
    reply->set_token("0");
  }
}

void LeagueFetcher::LoginUserAccount(
    const leagueservice::LoginUserAccountRequest *request,
    leagueservice::AuthToken *reply) {
  try {
    auto *connection = psql_fetcher_->GetCurrentConnection();
    pqxx::work W{*connection};
    const std::string get_user_account_id =
        fmt::format("SELECT id from user_account where account_username={} and "
                    "account_password={};",
                    W.quote(request->username()), W.quote(request->password()));
    pqxx::row id_row = W.exec1(get_user_account_id);
    std::string generated_token = get_uuid();
    const std::string insert_auth_token = fmt::format(
        "INSERT into user_auth_token(user_account_id, token) values({}, {});",
        id_row[0].as<int>(), W.quote(generated_token));
    W.exec0(insert_auth_token);
    W.commit();
    reply->set_token(generated_token);
  } catch (std::exception const &e) {
    reply->set_token("0");
  }
}

void LeagueFetcher::CreateLeague(
    const leagueservice::CreateLeagueRequest *request,
    leagueservice::CreateLeagueResponse *reply) {
  try {
    int user_account_id =
        auth_token_to_account_id(request->auth_token().token());
    if (!user_account_id) {
      return;
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
        W.quote(request->league_name()), league_settings_id,
        draft_row[0].as<int>());
    pqxx::row league_row = W.exec1(insert_league);
    W.commit();
    reply->set_league_id(league_row[0].as<int>());
  } catch (std::exception const &e) {
    reply->set_league_id(0);
  }
}

void LeagueFetcher::JoinLeague(const leagueservice::JoinLeagueRequest *request,
                               leagueservice::DefaultResponse *reply) {
  try {
    if (request->league_id() < 1) {
      reply->set_message("ERROR: Invalid league id.");
      return;
    }
    int user_account_id =
        auth_token_to_account_id(request->auth_token().token());
    if (!user_account_id) {
      reply->set_message("ERROR: Invalid authentication token.");
      return;
    }
    AddLeagueMember(user_account_id, request->league_id());
  } catch (std::exception const &e) {
    reply->set_message("ERROR: Internal error.");
  }
}

void LeagueFetcher::AddLeagueMember(int user_account_id, int league_id) {
  try {
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
  } catch (std::exception const &e) {
  }
}

void LeagueFetcher::MakeDraftPick(
    const leagueservice::DraftPickRequest *request,
    leagueservice::DefaultResponse *reply) {
  if (request->league_id() < 1) {
    reply->set_message("ERROR: Invalid league id.");
    return;
  }
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    reply->set_message("ERROR: Invalid authentication token.");
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_draft_id = fmt::format(
      "SELECT draft_id from league where id={};", request->league_id());
  pqxx::row draft_row = W.exec1(get_draft_id);
  const std::string insert_draft_pick = fmt::format(
      "INSERT into draft_selection(pick_number, player_id, draft_id, "
      "user_account_id) values({}, {}, {}, {});",
      request->pick_number(), request->player_selected_id(),
      draft_row[0].as<int>(), user_account_id);
  W.exec0(insert_draft_pick);
  const std::string insert_roster_member = fmt::format(
      "INSERT into roster_member(player_id, user_account_id, league_id, "
      "status, playable_positions) values({}, {}, {}, {}, {});",
      request->player_selected_id(), user_account_id, request->league_id(),
      W.quote("available"), W.quote(request->positions()));
  W.exec0(insert_roster_member);
  W.commit();
}

void LeagueFetcher::UpdateLineup(
    const leagueservice::UpdateLineupRequest *request,
    leagueservice::DefaultResponse *reply) {
  if (request->lineup_size() == 0) {
    reply->set_message("ERROR: Invalid lineup slot list.");
    return;
  }
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    reply->set_message("ERROR: Invalid authentication token.");
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string update_slot = "UPDATE lineup_slot SET player_id={} WHERE "
                                  "id={} AND user_account_id={};";
  for (const auto &slot : request->lineup()) {
    // TODO: Add some strict checks here to ensure that the position isn't being
    // changed with the new player set.
    const std::string &update_slot_player = fmt::format(
        update_slot, slot.player_id(), slot.lineup_slot_id(), user_account_id);
    W.exec0(update_slot_player);
  }
  W.commit();
}

void LeagueFetcher::GetBasicUserInformation(
    const leagueservice::AuthToken *request,
    leagueservice::BasicUserInformation *reply) {
  int user_account_id = auth_token_to_account_id(request->token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_basic_info =
      fmt::format("SELECT account_username, email, first_name, last_name from "
                  "user_account where id={};",
                  user_account_id);
  pqxx::row info_row = W.exec1(get_basic_info);
  W.commit();
  reply->set_username(info_row[0].as<std::string>());
  reply->set_email(info_row[1].as<std::string>());
  reply->set_first_name(info_row[2].as<std::string>());
  reply->set_last_name(info_row[3].as<std::string>());
}

void LeagueFetcher::GetMatchup(const leagueservice::MatchupRequest *request,
                               leagueservice::MatchupResponse *reply) {
  if (request->league_id() < 1 || request->week_number() < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_matchup = fmt::format(
      "SELECT user_1_id, user_2_id, score_1, score_2, ties_count, "
      "date_start, matchup_tag, matchup_id from matchup where user_1_id={} or "
      "user_id_2={} and week_num={} and league_id={};",
      user_account_id, user_account_id, request->week_number(),
      request->league_id());
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
  reply->set_username_1(username_1_row[0].as<std::string>());
  reply->set_username_2(username_2_row[0].as<std::string>());
  reply->set_score_1(matchup_row[2].as<int>());
  reply->set_score_2(matchup_row[3].as<int>());
  reply->set_ties_count(matchup_row[4].as<int>());
  reply->set_date_start(matchup_row[5].as<std::string>());
  reply->set_matchup_tag(matchup_row[6].as<std::string>());
  reply->set_matchup_id(matchup_row[7].as<int>());
}

void LeagueFetcher::GetMatch(const leagueservice::MatchRequest *request,
                             leagueservice::MatchResponse *reply) {
  if (request->match_id() < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_match = fmt::format(
      "SELECT user_id_1, user_id_2, match_date from match where match_id={};",
      request->match_id());
  pqxx::row match_row = W.exec1(get_match);
  W.commit();
  reply->set_user_id_1(match_row[0].as<int>());
  reply->set_user_id_2(match_row[1].as<int>());
  reply->set_match_date(match_row[2].as<std::string>());
}

void LeagueFetcher::GetLineup(const leagueservice::LineupRequest *request,
                              leagueservice::LineupResponse *reply) {
  if (request->match_id() < 1) {
    return;
  }
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_lineup =
      fmt::format("SELECT id, position, player_id, user_account_id from "
                  "lineup_slot where match_id={} and user_account_id={};",
                  request->match_id(), user_account_id);
  pqxx::result lineup_result = W.exec(get_lineup);
  W.commit();

  // Go through each lineup slot and add it to the result tuple.
  int result_user_id;
  if (lineup_result.empty() || lineup_result[3].empty()) {
    return;
  } else {
    result_user_id = lineup_result[0][3].as<int>();
  }
  reply->set_user_id(result_user_id);
  for (const auto &lineup_row : lineup_result) {
    auto *lineup_slot = reply->add_lineup();
    lineup_slot->set_lineup_slot_id(lineup_row[0].as<int>());
    lineup_slot->set_position(lineup_row[1].as<std::string>());
    lineup_slot->set_player_id(lineup_row[2].as<int>());
  }
}

void LeagueFetcher::GetRoster(const leagueservice::RosterRequest *request,
                              leagueservice::RosterResponse *reply) {
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_roster = fmt::format(
      "SELECT player_id, status, playable_positions, id from "
      "roster_member where user_account_id={} and league_id={}{}",
      user_account_id, request->league_id(),
      request->status_filter().empty()
          ? ";" // Only add filter constraint if it was provided.
          : fmt::format(" and status={};", request->status_filter()));
  pqxx::result roster_result = W.exec(get_roster);
  W.commit();
  for (const auto &roster_row : roster_result) {
    auto *roster_info = reply->add_roster();
    roster_info->set_player_id(roster_row[0].as<int>());
    roster_info->set_status(roster_row[1].as<std::string>());
    roster_info->set_playable_positions(roster_row[2].as<std::string>());
    roster_info->set_roster_member_id(roster_row[3].as<int>());
  }
}

void LeagueFetcher::GetLeaguesForMember(
    const leagueservice::LeaguesForMemberRequest *request,
    leagueservice::LeaguesForMemberResponse *reply) {
  int user_account_id = auth_token_to_account_id(request->auth_token().token());
  if (!user_account_id) {
    return;
  }
  auto *connection = psql_fetcher_->GetCurrentConnection();
  pqxx::work W{*connection};
  const std::string get_leagues =
      fmt::format("SELECT league_id from league_membership where "
                  "user_account_id={} and season_year={};",
                  user_account_id, request->season_year());
  pqxx::result league_result = W.exec(get_leagues);
  W.commit();
  for (const auto &league_row : league_result) {
    const std::string get_league_name = fmt::format(
        "SELECT league_name from league where id={};", league_row[0].as<int>());
    pqxx::row name_row = W.exec1(get_league_name);
    auto *league_description = reply->add_league_descriptions();
    league_description->set_league_id(league_row[0].as<int>());
    league_description->set_league_name(name_row[0].as<std::string>());
  }
}

int LeagueFetcher::init_league_settings(int commissioner_id) {
  try {
    auto *connection = psql_fetcher_->GetCurrentConnection();
    pqxx::work W{*connection};
    // Insert default waiver_settings.
    const std::string insert_waiver_settings =
        "INSERT into waiver_settings(waiver_delay, waiver_type) values(2, "
        "'standard') RETURNING id;";
    pqxx::row waiver_row = W.exec1(insert_waiver_settings);
    // Insert default transaction_settings.
    const std::string insert_transaction_settings =
        "INSERT into transaction_settings(trade_review_type, "
        "trade_review_time, "
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
  } catch (std::exception const &e) {
    return 0;
  }
}

int LeagueFetcher::auth_token_to_account_id(std::string token) {
  try {
    auto *connection = psql_fetcher_->GetCurrentConnection();
    pqxx::work W{*connection};
    const std::string sql_select = fmt::format(
        "SELECT user_account_id from user_auth_token WHERE token={};",
        W.quote(token));
    pqxx::row r = W.exec1(sql_select);
    W.commit();
    if (r.empty()) {
      return 0;
    }
    return r[0].as<int>();
  } catch (std::exception const &e) {
    return 0;
  }
}
} // namespace fantasy_ball