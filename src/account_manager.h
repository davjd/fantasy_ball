#ifndef ACCOUNT_MANAGER_H_
#define ACCOUNT_MANAGER_H_

#include <memory>
#include <wx/config.h>

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
  void SaveToken(const std::string& token);

  // Removes the active session.
  void ResetToken();

private:
  std::unique_ptr<wxConfig> config_;
  const std::string app_name_;
};

} // namespace fantasy_ball

#endif // ACCOUNT_MANAGER_H_