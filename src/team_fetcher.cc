#include "team_fetcher.h"

#include <nlohmann/json.hpp>
#include <string>

#include "curl_fetch.h"
#include "player_fetcher.h"

namespace fantasy_ball {
const std::string TeamFetcher::base_url_ =
    "https://api.mysportsfeeds.com/<version>/pull/nba/<season>/date/"
    "<date>/games.json";
// e.g.
// https://api.mysportsfeeds.com/v2.1/pull/nba/2020-2021-regular/date/20210319/games.json

TeamFetcher::TeamFetcher(CurlFetch *curl_fetch) : curl_fetch_(curl_fetch) {}

TeamFetcher::~TeamFetcher() {}

std::vector<TeamFetcher::GameMatchup>
TeamFetcher::GetGameReferences(endpoint::Options *options) {
  using json = nlohmann::json;
  std::vector<GameMatchup> matchups;
  const std::string endpoint_url = construct_endpoint_url(options);
  std::string content = curl_fetch_->GetContent(endpoint_url);
  if (curl_fetch_->curl_ret()) {
    return matchups;
  }
  if (!json::accept(content)) {
    return matchups;
  }
  json data = json::parse(content);
  if (!data.contains("games")) {
    return matchups;
  }
  const auto &games = data["games"];
  for (const auto &game : games) {
    const auto &game_matchup = GameMatchup::deserialize_json(game);
    if (game_matchup.event_id == -1) {
      continue;
    }
    matchups.push_back(game_matchup);
  }
  return matchups;
}

std::string TeamFetcher::construct_endpoint_url(endpoint::Options *options) {
  const std::string version = options->version;
  const std::string season_start = options->season_start;
  const std::string date = options->date;
  return replace(replace(replace(base_url_, "<version>", version),
                         "<season-start>", season_start),
                 "<date>", date);
}
} // namespace fantasy_ball