#include "account_manager.h"

#include <wx/string.h>

namespace fantasy_ball {

// Static test rosters for the demo.
std::vector<std::pair<std::string, std::string>> AccountManager::kRoster1 = {
    {"Luka", "Doncic"},      {"Collin", "Sexton"},    {"Jayson", "Tatum"},
    {"Bojan", "Bogdanovic"}, {"Danilo", "Gallinari"}, {"James", "Harden"},
    {"Norman", "Powell"},    {"Darius", "Bazley"},    {"Derrick", "Rose"},
    {"Deandre", "Ayton"},    {"Christian", "Wood"},   {"Trae", "Young"},
    {"Zach", "Lavine"}};

std::vector<std::pair<std::string, std::string>> AccountManager::kRoster2 = {
    {"Chris", "Paul"},       {"Jimmy", "Butler"},    {"Brandon", "Ingram"},
    {"John", "Collins"},     {"Demar", "Derozan"},   {"Tobias", "Harris"},
    {"Nikola", "Vucevic"},   {"Jerami", "Grant"},    {"Eric", "Bledsoe"},
    {"Donte", "Divincenzo"}, {"Duncan", "Robinson"}, {"Chris", "Boucher"},
    {"Kelly", "Olynyk"}};

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

bool AccountManager::HasALeague() { return config_->Exists("league_id"); }

void AccountManager::SaveToken(const std::string &token) {
  config_->Write("active_session_id", wxString(token));
}

void AccountManager::ResetToken() { config_->DeleteAll(); }

wxConfig *AccountManager::GetConfig() { return config_.get(); }

void AccountManager::SaveLeague(int league_id) {
  config_->Write("league_id", league_id);
}

int AccountManager::GetLeague() { return config_->Read("league_id", (int)0); }

void AccountManager::SetRoster(
    const std::vector<leagueservice::RosterInfo> &roster) {
  current_roster_ = roster;
}

std::vector<leagueservice::RosterInfo> AccountManager::GetRoster() {
  return current_roster_;
}

std::pair<fantasy_ball::TournamentManager::UserRoster,
          fantasy_ball::TournamentManager::UserRoster>
AccountManager::GetTestRosters(
    fantasy_ball::FantasyServiceClient *fantasy_client) {
  fantasy_ball::TournamentManager::UserRoster user_1 = {};
  fantasy_ball::TournamentManager::UserRoster user_2 = {};
  for (const auto &roster : kRoster1) {
    auto member =
        fantasy_client->GetPlayerDescription(roster.first, roster.second);
    user_1.roster.push_back(member);
  }

  for (const auto &roster : kRoster2) {
    auto member =
        fantasy_client->GetPlayerDescription(roster.first, roster.second);
    user_2.roster.push_back(member);
  }
  return std::make_pair(user_1, user_2);
}
} // namespace fantasy_ball