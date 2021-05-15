#ifndef ACCOUNT_MANAGER_H_
#define ACCOUNT_MANAGER_H_

#include <memory>
#include <utility>
#include <wx/config.h>

#include "fantasy_service_client.h"
#include "tournament_manager.h"

namespace fantasy_ball {

// This class will handle an account for a Fantasy Ball League instance.
class AccountManager {
public:
  AccountManager(const std::string &app_name);
  ~AccountManager() = default;

  // Get the user account id of this user.
  int GetUserAccountId();

  // Get the session id for the active session.
  std::string GetSessionId();

  // Checks if the user has a session stored.
  bool HasSession();

  // Returns the config for the given App session.
  wxConfig *GetConfig();

  // Stores the token for the account session.
  void SaveToken(const std::string &token);

  // Removes the active session.
  void ResetToken();

  // NOTE: This only used for the demo, since we don't currently (fully) support
  // draft picks, and other league features.
  static std::pair<fantasy_ball::TournamentManager::UserRoster,
                   fantasy_ball::TournamentManager::UserRoster>
  GetTestRosters(fantasy_ball::FantasyServiceClient *fantasy_client);

private:
  std::unique_ptr<wxConfig> config_;
  const std::string app_name_;

  static std::vector<std::pair<std::string, std::string>> kRoster1;
  static std::vector<std::pair<std::string, std::string>> kRoster2;
};

} // namespace fantasy_ball

#endif // ACCOUNT_MANAGER_H_