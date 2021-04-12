#include <iostream>
#include <memory>

#include "curl_fetch.h"
#include "player_fetcher.h"
#include "team_fetcher.h"
#include "util.h"

using namespace fantasy_ball;

int main() {
  // Example of how the data fetchers will be utilized:
  // 1. Choose a specific date.
  // 2. Initialize all the fetchers needed.
  // 3. Use the main data fetcher (player fetcher) to retrieve informaiton for a
  // specific player.
  endpoint::Options options = {};
  options.date = "20210410";
  options.season_start = "2020-2021-regular";
  options.version = "v2.1";
  std::unique_ptr<CurlFetch> curl_fetch = std::make_unique<CurlFetch>();
  curl_fetch->Init();
  std::unique_ptr<TeamFetcher> team_fetcher =
      std::make_unique<TeamFetcher>(curl_fetch.get());
  std::unique_ptr<PlayerFetcher> player_fetcher =
      std::make_unique<PlayerFetcher>(curl_fetch.get(), team_fetcher.get(),
                                      &options);
  std::cout << "date: " << player_fetcher->GetDefaultOptions().date << "\n";

  std::cout << "Key: " << curl_fetch->Key() << "\n";

  PlayerFetcher::PlayerIdentity player_info = {};
  player_info.first_name = "Mikal";
  player_info.last_name = "Bridges";

  bool added =
      player_fetcher->AddPlayer(player_info.first_name, player_info.last_name);
  if (!added) {
    std::cout << "Couldn't add player to the fetcher batch." << std::endl;
    return 0;
  }
  std::cout << "Successfully add player to the fetcher batch." << std::endl;

  const auto &log = player_fetcher->GetPlayerLog(player_info);

  char *url = NULL;
  curl_easy_getinfo(curl_fetch.get(), CURLINFO_EFFECTIVE_URL, &url);
  std::cout << "Ret: " << curl_fetch->curl_ret() << ", URL: " << url
            << std::endl;

  if (log.player_info.id < 0) {
    std::cout << "Couldn't retrieve daily player log. Error: "
              << log.player_info.id << std::endl;
    return 0;
  }
  std::cout << log.player_info.first_name << " " << log.player_info.last_name
            << " (" << log.player_info.id << ")" << std::endl;
  std::cout << "Points: " << log.player_log.points
            << ", Rebounds: " << log.player_log.total_rebounds << std::endl;
  std::cout << "(" << log.game_info.home_team << ") "
            << log.game_info.home_score << " - (" << log.game_info.away_team
            << ") " << log.game_info.away_score << std::endl;
  return 0;
}