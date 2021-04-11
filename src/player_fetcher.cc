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
const std::string PlayerFetcher::base_url_ =
    "https://api.mysportsfeeds.com/<version>/pull/nba/<season-start>/date/"
    "<date>/player_gamelogs.json?";

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

bool PlayerFetcher::AddPlayer(const std::string &fname,
                              const std::string &lname, const int &id,
                              endpoint::Options *options) {
  const auto &used_options =
      (options != nullptr ? GetDefaultOptions() : *options);
  bool added = false;
  if (fname.empty() && lname.empty() && id == -1) {
    return added;
  }
  if (used_options.strict_search && id == -1) {
    // Strict search requires that we provide an id number of the player we're
    // searching, to ensure that we actually find that single player.
    return added;
  }

  PlayerIdentity info = {};
  info.first_name = fname;
  info.last_name = lname;
  info.id = id;
  for (auto log_fetch : player_log_fetches_) {
    if (log_fetch.fetch_options == used_options) {
      log_fetch.roster = std::vector(1, info);
      added = true;
      break;
    }
  }

  if (!added) {
    // We need to create another fetch config for with the new options.
    PlayerLogFetch log_fetch = {};
    log_fetch.fetch_options = used_options;
    log_fetch.roster = std::vector(1, info);
    added = true;
  }
  return added;
}

// Set a roster that we'll do fetch calls on. The roster should be a list of
// player ids.
void PlayerFetcher::AddToRoster(std::vector<int> roster,
                                endpoint::Options *options) {
  auto used_options = (options != nullptr ? GetDefaultOptions() : *options);
  auto it = find_if(player_log_fetches_.begin(), player_log_fetches_.end(),
                    [&](const PlayerLogFetch log_fetch) {
                      return log_fetch.fetch_options == used_options;
                    });
  if (it != player_log_fetches_.end()) {
    for (const int &id : roster) {
      PlayerIdentity info = {};
      info.id = id;
      it->roster.push_back(info);
    }
  } else {
    PlayerLogFetch log_fetch = {};
    for (const int &id : roster) {
      PlayerIdentity info = {};
      info.id = id;
      log_fetch.roster.push_back(info);
    }
    player_log_fetches_.push_back(log_fetch);
  }
}

PlayerFetcher::DailyPlayerLog
PlayerFetcher::GetPlayerLog(const int &id, endpoint::Options *options) {
  if (id == -1) {
    return DailyPlayerLog();
  }

  auto used_options = (options != nullptr ? GetDefaultOptions() : *options);

  // Check if we already have this player log (with the given options) inside
  // the cache. Avoids doing curl calls everytime.
  auto id_iter = cache_.find(id);
  if (id_iter == cache_.end()) {
    // Do API call to retrieve the daily log.
    auto daily_player_log = retrieve_daily_player_log(id, &used_options);
    cache_[id] = std::vector(1, std::make_pair(used_options, daily_player_log));
    return daily_player_log;
  } else {
    for (const auto &log_pair : id_iter->second) {
      if (log_pair.first == used_options) {
        return log_pair.second;
      }
    }
    // This means we have a log (could be multiple) for the given player but not
    // with the given fetch options (e.g. could be for a different date).
    auto daily_player_log = retrieve_daily_player_log(id, &used_options);
    id_iter->second.push_back(std::make_pair(used_options, daily_player_log));
    return daily_player_log;
  }
}

std::unordered_map<int, PlayerFetcher::DailyPlayerLog>
PlayerFetcher::GetRosterLog(endpoint::Options *options) {
  std::unordered_map<int, DailyPlayerLog> daily_logs;
  // auto used_options = (options != nullptr ? GetDefaultOptions() : *options);
  // TODO: Complete this to work with the player_log_fetches_.
  return daily_logs;
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

std::string PlayerFetcher::make_player_list_url() {
  // TODO: Decide if we want to maintain log_fetches_, since it'll require
  // updating this and other functions.
  if (player_log_fetches_.size() == 0) {
    return "";
  }
  std::string players_url = "player=";
  for (const auto &player : player_log_fetches_.back().roster) {
    add_player_to_list_url(player, &players_url);
  }
  return players_url;
}

std::string
PlayerFetcher::make_player_list_url(PlayerFetcher::PlayerIdentity player) {
  std::string player_url = "player=";
  add_player_to_list_url(player, &player_url);
  return player_url;
}

void PlayerFetcher::add_player_to_list_url(PlayerFetcher::PlayerIdentity player,
                                           std::string *player_list_url) {
  if (player.id != -1) {
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
  return replace(replace(replace(base_url_, "<version>", version),
                         "<season-start>", season_start),
                 "<date>", date);
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

PlayerFetcher::DailyPlayerLog
PlayerFetcher::retrieve_daily_player_log(const int &id,
                                         endpoint::Options *options) {
  PlayerIdentity player = {};
  player.id = id;

  // Construct endpoint url and do curl operation.
  const std::string daily_log_endpoint_url =
      make_base_daily_log_url(options) + make_player_list_url(player);
  std::string json_content = curl_fetch_->GetContent(daily_log_endpoint_url);

  // Check if we had an error during the curl call.
  if (curl_fetch_->curl_ret()) {
    return DailyPlayerLog();
  }

  // Create the daily player log object by reading the json content response
  // returned by the MySportsFeed endpoint.
  auto used_options = (options != nullptr ? GetDefaultOptions() : *options);
  auto daily_player_logs = construct_player_logs(json_content, &used_options);
  // We should only have on log since we requested only one player id.
  if (daily_player_logs.size() == 1) {
    return daily_player_logs.front();
  }
  return DailyPlayerLog();
}

endpoint::Options PlayerFetcher::GetDefaultOptions() { return options_; }
} // namespace fantasy_ball