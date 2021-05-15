#include "player_fetcher.h"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <vector>

#include "curl_fetch.h"
#include "util.h"

namespace fantasy_ball {
const std::string PlayerFetcher::kDefaultVersion = "v2.1";
const std::string PlayerFetcher::kDefaultSeasonStart = "2020-2021-regular";
const std::string PlayerFetcher::kDefaultDate = "20210320";
const bool PlayerFetcher::kDefaultStrictSearch = true;
const std::string PlayerFetcher::kDailyPlayerLogUrl =
    "https://api.mysportsfeeds.com/<version>/pull/nba/<season-start>/date/"
    "<date>/player_gamelogs.json?";
const std::string PlayerFetcher::kPlayerInfoUrl =
    "https://scrambled-api.mysportsfeeds.com/<version>/pull/nba/"
    "players.json?"; // player=jordan-poole

PlayerFetcher::PlayerFetcher(CurlFetch *curl_fetch, TeamFetcher *team_fetcher,
                             endpoint::Options *options)
    : curl_fetch_(curl_fetch), team_fetcher_(team_fetcher) {
  if (options != nullptr) {
    options_ = *options;
  } else {
    endpoint::Options options = {};
    options.date = kDefaultDate;
    options.season_start = kDefaultSeasonStart;
    options.strict_search = kDefaultStrictSearch;
    options.version = kDefaultVersion;
    options_ = options;
  }
}

PlayerFetcher::~PlayerFetcher() {}

bool PlayerFetcher::AddPlayer(const PlayerFetcher::PlayerInfoShort &player_info,
                              endpoint::Options *options) {
  const auto &used_options =
      (options == nullptr ? GetDefaultOptions() : *options);
  bool added = false;
  if (player_info.is_empty()) {
    return added;
  }
  if (used_options.strict_search && player_info.id == -1) {
    // Strict search requires that we provide an id number of the player we're
    // searching, to ensure that we actually find that single player.
    return added;
  }

  for (auto log_fetch : player_log_fetches_) {
    if (log_fetch.fetch_options == used_options) {
      log_fetch.roster = std::vector(1, player_info);
      added = true;
      break;
    }
  }

  if (!added) {
    // We need to create another fetch config for with the new options.
    PlayerLogFetch log_fetch = {};
    log_fetch.fetch_options = used_options;
    log_fetch.roster = std::vector(1, player_info);
    player_log_fetches_.push_back(log_fetch);
    added = true;
  }
  return added;
}

void PlayerFetcher::AddToRoster(
    const std::vector<PlayerFetcher::PlayerInfoShort> &roster,
    endpoint::Options *options) {
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  auto it = find_if(player_log_fetches_.begin(), player_log_fetches_.end(),
                    [&](const PlayerLogFetch log_fetch) {
                      return log_fetch.fetch_options == used_options;
                    });
  if (it != player_log_fetches_.end()) {
    it->roster.insert(std::end(it->roster), std::begin(roster),
                      std::end(roster));
  } else {
    PlayerLogFetch log_fetch = {};
    log_fetch.fetch_options = used_options;
    log_fetch.roster.insert(std::end(log_fetch.roster), std::begin(roster),
                            std::end(roster));
    player_log_fetches_.push_back(log_fetch);
  }
}

PlayerFetcher::DailyPlayerLog
PlayerFetcher::GetPlayerLog(const PlayerFetcher::PlayerInfoShort &player,
                            endpoint::Options *options) {
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  if (used_options.strict_search && player.id == -1) {
    return DailyPlayerLog::MakeFaultyLog(1);
  }

  // We can only utilize the cache when we know the player id. Therefore, we
  // skip the cache and retrieve the data for fetch requests with players
  // without an id (they only have their name filled out).
  if (player.id == -1) {
    return retrieve_daily_player_log(player, &used_options);
  }

  // If we have an id, we check if we already have this player log (with the
  // given options) inside the cache. Avoids doing curl calls everytime.
  auto id_iter = cache_.find(player.id);
  if (id_iter == cache_.end()) {
    // Do API call to retrieve the daily log.
    auto daily_player_log = retrieve_daily_player_log(player, &used_options);
    cache_[daily_player_log.player_info.id] =
        std::vector(1, std::make_pair(used_options, daily_player_log));
    return daily_player_log;
  } else {
    for (const auto &log_pair : id_iter->second) {
      if (log_pair.first == used_options) {
        return log_pair.second;
      }
    }
    // This means we have a log (could be multiple) for the given player but not
    // with the given fetch options (e.g. could be for a different date).
    auto daily_player_log = retrieve_daily_player_log(player, &used_options);
    id_iter->second.push_back(std::make_pair(used_options, daily_player_log));
    return daily_player_log;
  }
}

std::vector<PlayerFetcher::DailyPlayerLog>
PlayerFetcher::GetRosterLog(endpoint::Options *options) {
  std::vector<DailyPlayerLog> daily_logs;
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  // Find the roster for the log fetch request that has the given options.
  auto it = find_if(player_log_fetches_.begin(), player_log_fetches_.end(),
                    [&](const PlayerLogFetch &log_fetch) {
                      return log_fetch.fetch_options == used_options;
                    });
  if (it == player_log_fetches_.end()) {
    return daily_logs;
  }

  // Find any player that isn't found in the cache, we will need to retrieve
  // them. Any other player can simply be returned.
  std::vector<PlayerInfoShort> missing_players;
  for (const auto &player : it->roster) {
    if (player.id == -1) {
      // We skip the cache for players without a valid id.
      daily_logs.push_back(retrieve_daily_player_log(player, &used_options));
      continue;
    }
    auto cache_entry_it = cache_.find(player.id);
    if (cache_entry_it == cache_.end()) {
      missing_players.push_back(player);
    } else {
      auto player_fetched_it = std::find_if(
          cache_entry_it->second.begin(), cache_entry_it->second.end(),
          [&](const std::pair<endpoint::Options, DailyPlayerLog>
                  player_fetched) {
            return player_fetched.first == used_options;
          });
      if (player_fetched_it == cache_entry_it->second.end()) {
        missing_players.push_back(player);
      } else {
        daily_logs.push_back(player_fetched_it->second);
      }
    }
  }

  // If all the requested logs were found in the cache, simply return them.
  if (missing_players.size() == 0) {
    return daily_logs;
  }
  // Retrieve the daily logs for the players not found in the cache.
  auto daily_player_logs =
      retrieve_daily_player_logs(missing_players, &used_options);
  // Store the players into the cache and add them to the returned vector.
  for (const auto &player : daily_player_logs) {
    auto cache_entry_it = cache_.find(player.player_info.id);
    auto fetched_player = std::make_pair(used_options, player);
    if (cache_entry_it == cache_.end()) {
      cache_[player.player_info.id] = std::vector(1, fetched_player);
    } else {
      cache_[player.player_info.id].push_back(fetched_player);
    }
    daily_logs.push_back(player);
  }
  return daily_logs;
}

void PlayerFetcher::GetPlayerInfoShort(
    PlayerFetcher::PlayerInfoShort *player_info, endpoint::Options *options) {
  if (player_info->is_empty()) {
    return;
  }
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  const std::string endpoint_url = make_base_player_info_url(&used_options) +
                                   make_player_list_url(*player_info);
  std::string json_content = curl_fetch_->GetContent(endpoint_url);

  // Check if we had an error during the curl call.
  if (curl_fetch_->curl_ret()) {
    return;
  }

  using json = nlohmann::json;
  // Check if valid json content.
  if (!json::accept(json_content)) {
    return;
  }
  json data = json::parse(json_content);
  if (data.empty() || !data.contains("players")) {
    return;
  }
  // We'll guess that the intended player is the first one returned.
  // TODO: Update this to verify that we selected the intended player.
  auto const &players = data["players"];
  if (players.size() == 0) {
    return;
  }
  player_info->read_json(players.front());
}

PlayerFetcher::DailyPlayerLog PlayerFetcher::RetrivePlayerLog(const int &id) {
  // TODO: Complete this function for non-cache retrievals.
  return DailyPlayerLog();
}

PlayerFetcher::DailyPlayerLog
PlayerFetcher::RetrivePlayerLog(const std::string &fname,
                                const std::string &lname) {
  // TODO: Complete this function for non-cache retrievals.
  return DailyPlayerLog();
}

std::string PlayerFetcher::make_player_list_url(
    const std::vector<PlayerFetcher::PlayerInfoShort> &roster) {
  std::string players_url = "player=";
  for (const auto &player : roster) {
    add_player_to_list_url(player, &players_url);
  }
  return players_url;
}

std::string PlayerFetcher::make_player_list_url(
    const PlayerFetcher::PlayerInfoShort &player) {
  std::string player_url = "player=";
  add_player_to_list_url(player, &player_url);
  return player_url;
}

void PlayerFetcher::add_player_to_list_url(
    const PlayerFetcher::PlayerInfoShort &player,
    std::string *player_list_url) {
  if (player.id != -1) {
    // TODO: Decide if we want to handle errors with 0 or negative numbers.
    *player_list_url += std::to_string(player.id);
  } else {
    if (!player.first_name.empty()) {
      (*player_list_url) += player.first_name + "-";
    }
    if (!player.last_name.empty()) {
      (*player_list_url) += player.last_name + "-";
    }

    if (player_list_url->back() == '-') {
      player_list_url->erase(player_list_url->size() - 1);
    }
  }
  (*player_list_url) += ",";
}

std::string PlayerFetcher::make_base_daily_log_url(endpoint::Options *options) {
  std::string version;
  std::string season_start;
  std::string date;
  if (options == nullptr) {
    version = options->version;
    season_start = options->season_start;
    date = options->date;
  } else {
    version = options_.version;
    season_start = options_.season_start;
    date = options_.date;
  }
  return replace(replace(replace(kDailyPlayerLogUrl, "<version>", version),
                         "<season-start>", season_start),
                 "<date>", date);
}

std::string
PlayerFetcher::make_base_player_info_url(endpoint::Options *options) {
  std::string version;
  if (options != nullptr) {
    version = options->version;
  } else {
    version = options_.version;
  }
  return replace(kPlayerInfoUrl, "<version>", version);
}

std::vector<PlayerFetcher::DailyPlayerLog>
PlayerFetcher::construct_player_logs(const std::string &curl_response,
                                     endpoint::Options *options) {
  std::vector<DailyPlayerLog> daily_player_logs;
  using json = nlohmann::json;
  // Check if valid json content.
  if (!json::accept(curl_response)) {
    return daily_player_logs;
  }
  json data = json::parse(curl_response);

  // Isolate each type of data for the player daily log.
  const auto &game_logs = get_game_logs(data);
  if (game_logs.empty()) {
    return daily_player_logs;
  }
  const auto &player_refs = get_player_references(data);
  if (player_refs.empty()) {
    return daily_player_logs;
  }
  const auto &game_refs = team_fetcher_->GetGameReferences(options);
  if (game_refs.empty()) {
    return daily_player_logs;
  }
  // For each game log, retrieve the other types of data. Skip incomplete game
  // logs that don't have corresponding data.
  for (const auto &game_log : game_logs) {
    if (!game_log.contains("player") || !game_log["player"].contains("id")) {
      continue;
    }
    DailyPlayerLog daily_player_log;
    const int &id = game_log["player"]["id"];
    const auto &player_ref = find_player_reference(player_refs, id);
    daily_player_log.player_info = PlayerIdentity::deserialize_json(player_ref);
    daily_player_log.player_log = PlayerLog::deserialize_json(game_log);

    // NOTE: The game/score data is retrieved using a different endpoint.
    // Therefore, we use the TeamFetcher to do get it, then we find the
    // corresponding game for this player log.
    auto it = find_if(game_refs.begin(), game_refs.end(),
                      [&](const TeamFetcher::GameMatchup &matchup) {
                        return matchup.event_id ==
                               daily_player_log.player_log.game_event_id;
                      });
    if (it == game_refs.end()) {
      continue;
    }
    daily_player_log.game_info = *it;
    daily_player_logs.push_back(daily_player_log);
  }
  return daily_player_logs;
}

nlohmann::json
PlayerFetcher::get_game_logs(const nlohmann::json &json_daily_log) {
  const auto &game_logs_iter = json_daily_log.find("gamelogs");
  if (game_logs_iter == json_daily_log.end()) {
    return nlohmann::json();
  }
  return *game_logs_iter;
}

nlohmann::json
PlayerFetcher::get_player_references(const nlohmann::json &json_daily_log) {
  if (!json_daily_log.contains("references")) {
    return nlohmann::json();
  }
  const auto &player_refs_iter =
      json_daily_log["references"].find("playerReferences");
  if (player_refs_iter == json_daily_log["references"].end()) {
    return nlohmann::json();
  }
  return *player_refs_iter;
}

nlohmann::json PlayerFetcher::get_game_log(const nlohmann::json &json_daily_log,
                                           int player_id) {
  const auto &game_logs = get_game_logs(json_daily_log);
  for (const auto &game_log : game_logs) {
    if (game_log.contains("player") && game_log["player"].contains("id") &&
        game_log["player"]["id"] == player_id) {
      return game_log;
    }
  }
  return nlohmann::json();
}

nlohmann::json
PlayerFetcher::get_player_reference(const nlohmann::json &json_daily_log,
                                    int player_id) {
  const auto &player_refs = get_player_references(json_daily_log);
  for (const auto &player_ref : player_refs) {
    if (player_ref.contains("id") && player_ref["id"] == player_id) {
      return player_ref;
    }
  }
  return nlohmann::json();
}

nlohmann::json PlayerFetcher::find_game_log(const nlohmann::json &game_logs,
                                            int player_id) {
  for (const auto &game_log : game_logs) {
    if (game_log.contains("player") && game_log["player"].contains("id") &&
        game_log["player"]["id"] == player_id) {
      return game_log;
    }
  }
  return nlohmann::json();
}

nlohmann::json
PlayerFetcher::find_player_reference(const nlohmann::json &player_refs,
                                     int player_id) {
  for (const auto &player_ref : player_refs) {
    if (player_ref.contains("id") && player_ref["id"] == player_id) {
      return player_ref;
    }
  }
  return nlohmann::json();
}

PlayerFetcher::DailyPlayerLog PlayerFetcher::retrieve_daily_player_log(
    const PlayerFetcher::PlayerInfoShort &player, endpoint::Options *options) {
  // Construct endpoint url and do curl operation.
  const std::string daily_log_endpoint_url =
      make_base_daily_log_url(options) + make_player_list_url(player);
  std::string json_content = curl_fetch_->GetContent(daily_log_endpoint_url);

  // Check if we had an error during the curl call.
  if (curl_fetch_->curl_ret()) {
    return DailyPlayerLog::MakeFaultyLog(2);
  }

  // Create the daily player log object by reading the json content response
  // returned by the MySportsFeed endpoint.
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  auto daily_player_logs = construct_player_logs(json_content, &used_options);
  // We should only have one log since we requested only one player id.
  if (daily_player_logs.size() == 1) {
    return daily_player_logs.front();
  }
  return DailyPlayerLog::MakeFaultyLog(3);
}

std::vector<PlayerFetcher::DailyPlayerLog>
PlayerFetcher::retrieve_daily_player_logs(
    const std::vector<PlayerFetcher::PlayerInfoShort> &roster,
    endpoint::Options *options) {
  // Construct endpoint url and do curl operation.
  const std::string daily_log_endpoint_url =
      make_base_daily_log_url(options) + make_player_list_url(roster);
  std::string json_content = curl_fetch_->GetContent(daily_log_endpoint_url);

  // Check if we had an error during the curl call.
  if (curl_fetch_->curl_ret()) {
    return std::vector<PlayerFetcher::DailyPlayerLog>();
  }

  // Create the daily player log object by reading the json content response
  // returned by the MySportsFeed endpoint.
  auto used_options = (options == nullptr ? GetDefaultOptions() : *options);
  auto daily_player_logs = construct_player_logs(json_content, &used_options);
  return daily_player_logs;
}

endpoint::Options PlayerFetcher::GetDefaultOptions() { return options_; }
} // namespace fantasy_ball