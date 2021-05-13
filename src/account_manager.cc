#include "account_manager.h"

#include <wx/string.h>

namespace fantasy_ball {

AccountManager::AccountManager(const std::string &app_name)
    : app_name_(app_name) {
  // Initialize the config.
  config_ = std::make_unique<wxConfig>(app_name);
}

int AccountManager::GetUserAccountId() {
  return config_->Read("user_account_id", (int)0);
}

std::string AccountManager::GetSessionId() {
  return config_->Read("active_session_id", "0").ToStdString();
}

bool AccountManager::HasSession() {
  return config_->Exists("active_session_id");
}

void AccountManager::SaveToken(const std::string &token) {
  config_->Write("active_session_id", wxString(token));
}

void AccountManager::ResetToken() { config_->DeleteAll(); }

wxConfig *AccountManager::GetConfig() { return config_.get(); }
} // namespace fantasy_ball