#include <iostream>
#include <memory>

#include "account_manager.h"
#include "fantasy_service_client.h"
#include "player_fetcher.h"
#include "util.h"

#include <grpcpp/create_channel.h>
#include <iostream>
#include <memory>

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
  auto account_manager = std::make_unique<AccountManager>("FantasyApp");
  auto fantasy_client = std::make_unique<FantasyServiceClient>(
      grpc::CreateChannel("localhost:50050",
                          grpc::InsecureChannelCredentials()),
      grpc::CreateChannel("localhost:50051",
                          grpc::InsecureChannelCredentials()));

  // Get the player id's for the fake test player data.
  auto rosters = AccountManager::GetTestRosters(fantasy_client.get());
  // Go through all the static roster members and insert each one as draft picks
  // for each user. But check if both are the same size.
  if (rosters.first.roster.size() != rosters.second.roster.size()) {
    std::cout << "Roster sizes did not match." << std::endl;
    return 0;
  }
  std::cout << rosters.first.roster[0].player_id << std::endl;
  // std::cout << "First roster: " << std::endl;
  // for (const auto &member : rosters.first.roster) {
  //   std::cout << member.first_name << " " << member.last_name << ": "
  //             << member.player_id << std::endl;
  // }

  // std::cout << "Second roster: " << std::endl;
  // for (const auto &member : rosters.second.roster) {
  //   std::cout << member.first_name << " " << member.last_name << ": "
  //             << member.player_id << std::endl;
  // }

  // Create fake users.
  std::string token_1 = fantasy_client->RegisterAccount(
      "lbj", "lbj@nba.com", "password", "leb", "jam");
  std::string token_2 = fantasy_client->RegisterAccount(
      "kd", "kd@nba.com", "password", "kev", "dur");

  std::cout << "TOKEN: " << token_1 << " : " << token_2 << std::endl;
  if (token_1.empty() || token_2.empty()) {
    std::cout << "Could not create users." << std::endl;
  }

  // Create a fake league.
  int league_id = fantasy_client->CreateLeague(token_1, "The Mock League");
  if (league_id == 0) {
    std::cout << "Could not create league." << std::endl;
    return 0;
  }

  // Add both users to the league.
  bool first_joined = fantasy_client->JoinLeague(token_1, league_id);
  bool second_joined = fantasy_client->JoinLeague(token_2, league_id);
  if (!first_joined || !second_joined) {
    std::cout << "Could not add league members." << std::endl;
    return 0;
  }

  // Mock a draft by inserting draft picks for both users.
  for (int i = 0; i < rosters.first.roster.size(); ++i) {
    // We alternate doing draft picks, mocking the actual draft.
    int base_pick = i * 2;
    fantasy_client->MakeDraftPick(token_1, base_pick + 1,
                                  rosters.first.roster[i].player_id, league_id,
                                  "");
    fantasy_client->MakeDraftPick(token_2, base_pick + 2,
                                  rosters.second.roster[i].player_id, league_id,
                                  "");
  }

  std::cout << "Created test data." << std::endl;
  return 0;
}