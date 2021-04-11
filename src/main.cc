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

  bool added = player_fetcher->AddPlayer("Mikal", "Bridges");
  if (!added) {
    std::cout << "Couldn't added player to the fetcher batch." << std::endl;
    return 0;
  }
  std::cout << "Successfully added player to the fetcher batch." << std::endl;
  


  return 0;
}