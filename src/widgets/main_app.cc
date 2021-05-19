// Main client application.

#include "account_manager.h"
#include "fantasy_service_client.h"
#include "util.h"
#include "wxglade_out.h"

#include <fmt/core.h>
#include <grpcpp/create_channel.h>
#include <wx/msgout.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
class FantasyApp : public wxApp {
public:
  virtual bool OnInit();
  virtual int OnExit();

private:
  fantasy_ball::AccountManager *account_manager_;
  fantasy_ball::FantasyServiceClient *fantasy_client_;

  // The different frames we'll be showing.
  LoginFrame *login_frame_;
  MatchupFrame *matchup_frame_;
  RegisterFrame *register_frame_;

  // TODO: It would be better if these were inside the frames' classes.
  void DisplayLoginErrorLabel(bool show);
  void DisplayRegisterErrorLabel(bool show);
  void ShowMainInterface();
  void ShowLoginInterface();
  void ShowRegisterInterface();

  // Custom event handlers.
  void OpenMainInterface(wxCommandEvent &event);
  void OpenLoginInterface(wxCommandEvent &event);
  void OpenRegisterInterface(wxCommandEvent &event);

  void LoginUser(wxCommandEvent &event);
  void RegisterUser(wxCommandEvent &event);
  void GetLineup(wxCommandEvent &event);
  void GetRoster(wxShowEvent &event);
};

enum { ID_Login_Frame = 1, ID_Register_Frame, ID_Matchup_Frame };
wxIMPLEMENT_APP(FantasyApp);
bool FantasyApp::OnInit() {
  // Initialize needed stuff.
  wxInitAllImageHandlers();
  account_manager_ = new fantasy_ball::AccountManager("FantasyApp");
  fantasy_client_ = new fantasy_ball::FantasyServiceClient(
      grpc::CreateChannel("localhost:50050",
                          grpc::InsecureChannelCredentials()),
      grpc::CreateChannel("localhost:50051",
                          grpc::InsecureChannelCredentials()));

  account_manager_->ResetToken();
  // Create all the UI frames.
  login_frame_ = new LoginFrame(this->GetTopWindow(), ID_Login_Frame,
                                wxT("Fantasy Ball League"));
  register_frame_ = new RegisterFrame(this->GetTopWindow(), ID_Register_Frame,
                                      wxT("Fantasy Ball League"));
  matchup_frame_ = new MatchupFrame(this->GetTopWindow(), ID_Matchup_Frame,
                                    wxT("Fantasy Ball League"));

  // Hide error labels.
  DisplayLoginErrorLabel(false);
  DisplayRegisterErrorLabel(true);

  // Bind events. We do this here because the parent can modify the children
  // widgets/frames.
  login_frame_->sign_in_button->Bind(wxEVT_BUTTON, &FantasyApp::LoginUser,
                                     this);
  login_frame_->register_button->Bind(wxEVT_BUTTON,
                                      &FantasyApp::OpenRegisterInterface, this);
  register_frame_->register_button->Bind(wxEVT_BUTTON,
                                         &FantasyApp::RegisterUser, this);
  matchup_frame_->Bind(wxEVT_SHOW, &FantasyApp::GetRoster, this);

  // Determine which initial page to show.
  if (account_manager_->HasSession()) {
    matchup_frame_->Show(true);
  } else {
    login_frame_->Show(true);
  }
  this->GetTopWindow()->Centre();
  return true;
}

void FantasyApp::OpenMainInterface(wxCommandEvent &event) {
  ShowMainInterface();
}

void FantasyApp::OpenLoginInterface(wxCommandEvent &event) {
  ShowLoginInterface();
}

void FantasyApp::OpenRegisterInterface(wxCommandEvent &event) {
  ShowRegisterInterface();
}

void FantasyApp::ShowMainInterface() {
  this->login_frame_->Show(false);
  this->register_frame_->Show(false);
  this->matchup_frame_->Show(true);
}

void FantasyApp::ShowLoginInterface() {
  this->login_frame_->Show(true);
  this->register_frame_->Show(false);
  this->matchup_frame_->Show(false);
}

void FantasyApp::ShowRegisterInterface() {
  this->login_frame_->Show(false);
  this->register_frame_->Show(true);
  this->matchup_frame_->Show(false);
}

void FantasyApp::LoginUser(wxCommandEvent &event) {
  // Use the input and send a login request to the service.
  auto username = this->login_frame_->username_input->GetValue().ToStdString();
  auto password = this->login_frame_->password_input->GetValue().ToStdString();
  if (username.empty() || password.empty()) {
    DisplayLoginErrorLabel(true);
    return;
  }
  // Send the request to the league server.
  auto token = fantasy_client_->Login(username, password);
  wxMessageOutput::Get()->Printf("Token[%i]: %s", token.empty(), token);
  if (token == "0") {
    // An error occured, so display error message.
    DisplayLoginErrorLabel(true);
  } else {
    // We must retrieve the league id, for now we'll select the first league
    // this user is in. We use a default year for now.
    auto league_descriptions =
        fantasy_client_->GetLeaguesForMember(token, "2020-2021");
    if (league_descriptions.empty()) {
      // TODO: We return an error if the user isn't in a league. Change this to
      // handle this case.
      wxMessageOutput::Get()->Printf("Found no leagues!");
      return;
    }
    wxMessageOutput::Get()->Printf("Saving league %d",
                                   league_descriptions.at(0).league_id());
    // Store the token and the league id for this session.
    account_manager_->SaveToken(token);
    account_manager_->SaveLeague(league_descriptions.at(0).league_id());

    wxMessageOutput::Get()->Printf("Successful login");
    DisplayLoginErrorLabel(false);
    ShowMainInterface();
  }
}

void FantasyApp::RegisterUser(wxCommandEvent &event) {
  auto username =
      this->register_frame_->username_input->GetValue().ToStdString();
  auto password =
      this->register_frame_->password_input->GetValue().ToStdString();
  auto email = this->register_frame_->email_input->GetValue().ToStdString();
  auto first_name =
      this->register_frame_->first_name_input->GetValue().ToStdString();
  auto last_name =
      this->register_frame_->last_name_input->GetValue().ToStdString();
  if (username.empty() || password.empty() || email.empty() ||
      first_name.empty() || last_name.empty()) {
    DisplayRegisterErrorLabel(true);
    return;
  } else {
    DisplayRegisterErrorLabel(false);
  }

  // Register the account, by sending request.
  auto token = fantasy_client_->RegisterAccount(username, email, password,
                                                first_name, last_name);
  wxMessageOutput::Get()->Printf("Token[%i]: %s", token.empty(), token);
  if (token.empty()) {
    // Display error message.
    DisplayRegisterErrorLabel(true);
    return;
  }

  // Check if a league id was provided.
  auto league_id =
      this->register_frame_->league_input->GetValue().ToStdString();
  if (!league_id.empty() && fantasy_ball::is_number(league_id)) {
    wxMessageOutput::Get()->Printf("Joining league [%i]: %s",
                                   std::stoi(league_id), token);
    bool successful_join =
        fantasy_client_->JoinLeague(token, std::stoi(league_id));
    if (!successful_join) {
      wxMessageOutput::Get()->Printf("Error joining league [%i]: %s",
                                     std::stoi(league_id), token);
    }
  }
  account_manager_->SaveToken(token);
  account_manager_->SaveLeague(std::stoi(league_id));
  DisplayRegisterErrorLabel(false);
  ShowMainInterface();
}

void FantasyApp::GetLineup(wxCommandEvent &event) {
  // TODO: Because the regular season is over, we will mock a date here, but
  // this should be easily modifiable.
  // fantasy_client_->AddPlayerToFetchBatch(member.player_id(), "20210410");
}

void FantasyApp::GetRoster(wxShowEvent &event) {
  if (!account_manager_->HasALeague()) {
    wxMessageOutput::Get()->Printf(
        "Found no league for user to retrieve roster for.");
    return;
  }
  if (account_manager_->GetRoster().empty()) {
    wxMessageOutput::Get()->Printf("Grabbing roster");
    auto roster = fantasy_client_->GetRoster(account_manager_->GetSessionId(),
                                             account_manager_->GetLeague());
    account_manager_->SetRoster(roster);
    wxMessageOutput::Get()->Printf("Got %d roster members", (int)roster.size());
  }
  for (const auto &member : account_manager_->GetRoster()) {
    auto description =
        fantasy_client_->GetPlayerDescription(member.player_id());
    // Dynamically create the widget container for each roster member.
    // TODO: Move this to a function, but would wait to figure out how we should
    // extend functionalities to the generated frame classes.
    wxBoxSizer *roster_member_sizer = new wxBoxSizer(wxVERTICAL);
    matchup_frame_->roster_list_sizer->Add(roster_member_sizer, 0,
                                           wxALIGN_CENTER_HORIZONTAL, 0);
    wxBoxSizer *labels_sizer = new wxBoxSizer(wxHORIZONTAL);
    roster_member_sizer->Add(labels_sizer, 0, wxEXPAND, 0);

    // Create a label to display the player's name, team, and position.
    const std::string name_display =
        fmt::format("{}. {}", description.first_name[0], description.last_name);
    wxStaticText *name_label = new wxStaticText(
        matchup_frame_->notebook_1_pane_1, wxID_ANY, wxString(name_display));
    name_label->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL, 0, wxT("")));
    labels_sizer->Add(name_label, 0, wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText *team_label =
        new wxStaticText(matchup_frame_->notebook_1_pane_1, wxID_ANY,
                         wxString(description.team));
    team_label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                               wxFONTWEIGHT_NORMAL, 0, wxT("")));
    labels_sizer->Add(team_label, 0, wxALIGN_CENTER_VERTICAL, 0);
    wxStaticText *position_label =
        new wxStaticText(matchup_frame_->notebook_1_pane_1, wxID_ANY,
                         wxString("- " + description.positions));
    position_label->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                   wxFONTWEIGHT_NORMAL, 0, wxT("")));
    labels_sizer->Add(position_label, 0, wxALIGN_CENTER_VERTICAL, 0);

    wxStaticLine *separator_line =
        new wxStaticLine(matchup_frame_->notebook_1_pane_1, wxID_ANY);
    roster_member_sizer->Add(separator_line, 0, wxEXPAND, 0);
    roster_member_sizer->Layout();
    matchup_frame_->roster_list_sizer->Layout();

  }
  this->GetTopWindow()->Layout();
}

void FantasyApp::DisplayLoginErrorLabel(bool show) {
  this->login_frame_->login_error_message_sizer->Show(show);
}

void FantasyApp::DisplayRegisterErrorLabel(bool show) {
  this->register_frame_->register_error_message_sizer->Show(show);
}

int FantasyApp::OnExit() {
  delete account_manager_;
  delete fantasy_client_;
  return 0;
}