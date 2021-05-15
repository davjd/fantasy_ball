#include "tournament_manager.h"

#include <algorithm>
#include <numeric>

namespace fantasy_ball {

const std::array<int, 10> TournamentManager::kAvailableLeagueSizes = {
    2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
const int TournamentManager::kRegularSeasonWeeks = 16;

const int TournamentManager::kPlayoffSeasonWeeks = 3;

const std::unordered_map<std::string, float>
    TournamentManager::kHeadToHeadScoreMap = {
        {"points", 1.0}, {"rebounds", 1.2}, {"assists", 1.5},
        {"blocks", 3},   {"steals", 3},     {"turnover", -1.0}};

const std::array<std::string, 9> TournamentManager::kCategoryScoreList = {
    "(FG%)", "(FT%)", "(3PTM)", "(PTS)", "(REB)",
    "(AST)", "(ST)",  "(BLK)",  "(TO)"};

const std::array<std::string, 10> TournamentManager::kAvailablePositions = {
    "PG", "SG", "G", "SF", "PF", "F", "C", "C", "UTIL", "UTIL"};

bool TournamentManager::IsValidTournamentSize(int size) {
  return std::find(kAvailableLeagueSizes.begin(), kAvailableLeagueSizes.end(),
                   size) != kAvailableLeagueSizes.end();
}

std::vector<std::vector<std::pair<int, int>>>
TournamentManager::ComputeTournamentMatchups(int league_size, int rounds) {
  std::vector<std::vector<std::pair<int, int>>> tournament_rounds;
  if (league_size < 2 || rounds < 1) {
    return tournament_rounds;
  }

  // Generate series from 0 to league_size.
  std::vector<int> players(league_size);
  std::iota(players.begin(), players.end(), 0);

  int num_players = players.size();
  int half = num_players / 2;
  // We'll only calculate the minimum required rounds (if greater than
  // league_size, we don't need to recomputer, we'll just repeat some rounds).
  int min_rounds = std::min(num_players - 1, rounds);

  // All indices except the first one.
  auto player_indices = std::vector<int>(players.begin() + 1, players.end());

  // Calculate round robin matches for each round.
  for (int round = 0; round < min_rounds; ++round) {
    std::vector<std::pair<int, int>> round_matches;

    // This copy has the updated indices that have been shifted one spot.
    std::vector<int> round_robin_indices = {0};
    round_robin_indices.insert(round_robin_indices.end(),
                               player_indices.begin(), player_indices.end());

    // We split each half and create the matches.
    auto first_half = std::vector<int>(round_robin_indices.begin(),
                                       round_robin_indices.begin() + half);
    auto second_half = std::vector<int>(round_robin_indices.begin() + half,
                                        round_robin_indices.end());
    std::reverse(second_half.begin(), second_half.end());
    for (int i = 0; i < first_half.size(); ++i) {
      round_matches.push_back(std::make_pair(first_half[i], second_half[i]));
    }

    // Shift the indices by one spot.
    std::rotate(player_indices.begin(), player_indices.begin() + 1,
                player_indices.end());
    // Add the calculated matches for this round.
    tournament_rounds.push_back(round_matches);
  }

  if (min_rounds == rounds) {
    return tournament_rounds;
  }
  // We don't recalculate if rounds > league_size - 1, we simply cycle the
  // previous rounds.
  int cycled_rounds = rounds - min_rounds;
  for (int i = 0; i < cycled_rounds; ++i) {
    tournament_rounds.push_back(tournament_rounds[i % min_rounds]);
  }
  return tournament_rounds;
}

float TournamentManager::CalculateHeadToHeadScore(
    std::vector<float> categories) {
  float total_points = 0.0;
  if (categories.size() != kHeadToHeadScoreMap.size()) {
    return 0.0;
  }
  auto iter = kHeadToHeadScoreMap.begin();
  for (int i = 0; i < categories.size(); ++i) {
    total_points += (iter->second * categories[i]);
    std::advance(iter, 1);
  }
  return total_points;
}
} // namespace fantasy_ball