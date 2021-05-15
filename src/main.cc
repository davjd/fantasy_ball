#include <iostream>
#include <memory>

#include "curl_fetch.h"
#include "player_fetcher.h"
#include "team_fetcher.h"
#include "util.h"

using namespace fantasy_ball;

void print_player_log_info(const PlayerFetcher::DailyPlayerLog &log) {
  if (log.player_info.id < 0) {
    std::cout << "Couldn't retrieve daily player log. Error: "
              << log.player_info.id << std::endl;
    return;
  }
  std::cout << log.player_info.first_name << " " << log.player_info.last_name
            << " (" << log.player_info.id << ")" << std::endl;
  std::cout << "Points: " << log.player_log.points
            << ", Rebounds: " << log.player_log.total_rebounds << std::endl;
  std::cout << "(" << log.game_info.home_team << ") "
            << log.game_info.home_score << " - (" << log.game_info.away_team
            << ") " << log.game_info.away_score << std::endl;
}

void print_player_info_short(
    const PlayerFetcher::PlayerInfoShort &player_info) {
  std::cout << player_info.first_name << " " << player_info.last_name << "("
            << player_info.id << ") : " << player_info.team << "("
            << player_info.team_id << ")" << std::endl;
}

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
  options.strict_search = false;
  std::unique_ptr<CurlFetch> curl_fetch = std::make_unique<CurlFetch>();
  curl_fetch->Init();
  std::unique_ptr<TeamFetcher> team_fetcher =
      std::make_unique<TeamFetcher>(curl_fetch.get());
  std::unique_ptr<PlayerFetcher> player_fetcher =
      std::make_unique<PlayerFetcher>(curl_fetch.get(), team_fetcher.get(),
                                      &options);
  std::cout << "date: " << player_fetcher->GetDefaultOptions().date << "\n";

  std::cout << "Key: " << curl_fetch->Key() << "\n";

  std::vector<PlayerFetcher::PlayerInfoShort> roster;
  roster.push_back({"Luka", "Doncic"});
  roster.push_back({"Collin", "Sexton"});
  roster.push_back({"Jayson", "Tatum"});
  roster.push_back({"Bojan", "Bogdanovic"});
  roster.push_back({"Danilo", "Gallinari"});
  roster.push_back({"James", "Harden"});
  roster.push_back({"Norman", "Powell"});
  roster.push_back({"Darius", "Bazley"});
  roster.push_back({"Derrick", "Rose"});
  roster.push_back({"Deandre", "Ayton"});
  roster.push_back({"Christian", "Wood"});
  roster.push_back({"Trae", "Young"});
  roster.push_back({"Zach", "Lavine"});

  roster.push_back({"Chris", "Paul"});
  roster.push_back({"Jimmy", "Butler"});
  roster.push_back({"Brandon", "Ingram"});
  roster.push_back({"John", "Collins"});
  roster.push_back({"Demar", "Derozan"});
  roster.push_back({"Tobias", "Harris"});
  roster.push_back({"Nikola", "Vucevic"});
  roster.push_back({"Jerami", "Grant"});
  roster.push_back({"Eric", "Bledsoe"});
  roster.push_back({"Donte", "Divincenzo"});
  roster.push_back({"Duncan", "Robinson"});
  roster.push_back({"Chris", "Boucher"});
  roster.push_back({"Kelly", "Olynyk"});

  // Update the roster's player info with their id and other data.
  {
    for (auto &player : roster) {
      player_fetcher->GetPlayerInfoShort(&player);
    }
    for (const auto &player : roster) {
      print_player_info_short(player);
    }
  }

  // Use the updated player infos to retrieve data.
  // First we need to add the players into a batch.

  // Getting data for just one player.
  // {
  //   bool added = player_fetcher->AddPlayer(roster.front());
  //   if (!added) {
  //     std::cout << "Couldn't add player to the fetcher batch." << std::endl;
  //     return 0;
  //   }
  //   std::cout << "Successfully added player to the fetcher batch." << std::endl;
  //   const auto &log = player_fetcher->GetPlayerLog(roster.front());
  //   print_player_log_info(log);
  // }

  // Get multiple players in one batch.
  {
    player_fetcher->AddToRoster(roster);
    const auto &logs = player_fetcher->GetRosterLog(&options);
    std::cout << "Number of logs: " << logs.size() << std::endl;
    for (const auto &player_log : logs) {
      print_player_log_info(player_log);
    }
  }
  return 0;
}