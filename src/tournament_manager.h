#ifndef TOURNAMENT_MANAGER
#define TOURNAMENT_MANAGER

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace fantasy_ball {
class TournamentManager {
public:
  // Valid leage sizes are 4,6,8,..,20.
  static const std::array<int, 10> kAvailableLeagueSizes;

  // Total weeks (of matchups) in the regular season.
  static const int kRegularSeasonWeeks;

  // Total weeks (of matchups) in the playoff season.
  static const int kPlayoffSeasonWeeks;

  // Head-to-head Matchup:
  // Points: 1 Rebounds: 1.2 Assists: 1.5 Blocks: 3 Steals: 3 Turnovers: -1
  static const std::unordered_map<std::string, float> kHeadToHeadScoreMap;

  // Category Matchup:
  // Field Goal Percentage (FG%), Free Throw Percentage (FT%), 3-pointers Made
  // (3PTM), Points Scored (PTS), Total Rebounds (REB), Assists (AST), Steals
  // (ST), Blocked Shots (BLK) and Turnovers (TO).
  static const std::array<std::string, 9> kCategoryScoreList;

  // Available positions in a given lineup.
  // Positions: PG, SG, G, SF, PF, F, C, C, UTIL, UTIL
  static const std::array<std::string, 10> kAvailablePositions;

  TournamentManager() = default;
  ~TournamentManager() = default;

  // Returns whether the size of a given tournament is valid.
  static bool IsValidTournamentSize(int size);

  // Returns a vector with matches for each round. Rounds are repeated for
  // rounds > league_size - 1. It is up to caller determine what user account
  // belongs to what index.
  //
  // Example: league_size=4, rounds=4 ---> {0,1,2,3}
  // Round 1:
  //  [0,3] [1,2] --> (0 vs. 3), (1 vs. 2).
  // Round 2:
  //  [0,1] [2,3] ...
  // Round 3:
  //  [0,2] [3,1] ...
  // Round 4:
  //  [0,3] [1,2] --> (0 vs. 3), (1 vs. 2) -> matches are repeated.
  std::vector<std::vector<std::pair<int, int>>>
  ComputeTournamentMatchups(int league_size, int rounds);

  // Returns the total fantasy points for a Head-To-Head match.
  // NOTE: Expects categories to contain the scores for each category in a
  // head-to-head score map. The list should follow the order of the
  // kHeadToHeadScoreMap categories (points->rebounds->assists, etc).
  float CalculateHeadToHeadScore(std::vector<float> categories);

  struct RosterMember {
    // TODO: This is pretty much a copy of PlayerInfoShort. Might be better to
    // put this inside a common/util file.
    RosterMember() = default;
    std::string first_name;
    std::string last_name;
    std::string team;
    int player_id;
    int team_id;
  };

  // Contains a short description of a player.
  struct UserRoster {
    UserRoster() = default;
    std::string username;
    int user_account_id;
    int league_id;
    std::vector<RosterMember> roster;
  };
};

} // namespace fantasy_ball

#endif // TOURNAMENT_MANAGER