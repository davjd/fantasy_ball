// Main client application.

#include "account_manager.h"
#include "fantasy_service_client.h"
#include "wxglade_out.h"

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

  void DisplayLoginErrorLabel(bool show);

  // Custom event handlers.
  void OpenMainInterface(wxCommandEvent &event);
  void OpenLoginInterface(wxCommandEvent &event);
  void OpenRegisterInterface(wxCommandEvent &event);

  void LoginUser(wxCommandEvent &event);
  void RegisterUser(wxCommandEvent &event);
};
// class MyFrame : public wxFrame {
// public:
//   MyFrame();

// private:
//   void OnHello(wxCommandEvent &event);
//   void OnExit(wxCommandEvent &event);
//   void OnAbout(wxCommandEvent &event);
// };
enum { ID_Login_Frame = 1, ID_Register_Frame, ID_Matchup_Frame };
wxIMPLEMENT_APP(FantasyApp);
bool FantasyApp::OnInit() {
  // Initialize needed stuff.
  wxInitAllImageHandlers();
  account_manager_ = new fantasy_ball::AccountManager("FantasyApp");
  fantasy_client_ = new fantasy_ball::FantasyServiceClient(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  // Create all the UI frames.
  login_frame_ = new LoginFrame(this->GetTopWindow(), ID_Login_Frame,
                                wxT("Fantasy Ball League"));
  DisplayLoginErrorLabel(false);
  register_frame_ = new RegisterFrame(this->GetTopWindow(), ID_Register_Frame,
                                      wxT("Fantasy Ball League"));
  matchup_frame_ = new MatchupFrame(this->GetTopWindow(), ID_Matchup_Frame,
                                    wxT("Fantasy Ball League"));

  // Bind events. We do this here because the parent can modify the children
  // widgets/frames.
  login_frame_->sign_in_button->Bind(wxEVT_BUTTON, &FantasyApp::LoginUser,
                                     this);
  login_frame_->register_button->Bind(wxEVT_BUTTON,
                                      &FantasyApp::OpenRegisterInterface, this);
  register_frame_->register_button->Bind(wxEVT_BUTTON,
                                         &FantasyApp::RegisterUser, this);

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
  this->login_frame_->Show(false);
  this->register_frame_->Show(false);
  this->matchup_frame_->Show(true);
  this->GetTopWindow()->Centre();
}

void FantasyApp::OpenLoginInterface(wxCommandEvent &event) {
  this->login_frame_->Show(true);
  this->register_frame_->Show(false);
  this->matchup_frame_->Show(false);
  this->GetTopWindow()->Centre();
}
void FantasyApp::OpenRegisterInterface(wxCommandEvent &event) {
  this->login_frame_->Show(false);
  this->register_frame_->Show(true);
  this->matchup_frame_->Show(false);
  this->GetTopWindow()->Centre();
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
  if (token == "0") {
    // An error occured, so display error message.
    DisplayLoginErrorLabel(true);
  } else {
    DisplayLoginErrorLabel(false);
  }
  wxMessageOutput::Get()->Printf("Token[%i]: %s", token.empty(), token);
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
    return;
  }
  // Register the account, by sending request.
  auto token = fantasy_client_->RegisterAccount(username, email, password,
                                                first_name, last_name);
  // TODO: Send JoinLeague request if league id was provided.
  wxMessageOutput::Get()->Printf("Token[%i]: %s", token.empty(), token);
}

void FantasyApp::DisplayLoginErrorLabel(bool show) {
  this->login_frame_->login_error_message_sizer->Show(show);
}

int FantasyApp::OnExit() {
  delete account_manager_;
  delete fantasy_client_;
  return 0;
}