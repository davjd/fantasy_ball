// Main client application.

#include "account_manager.h"
#include "fantasy_service_client.h"
#include "wxglade_out.h"

#include <grpcpp/create_channel.h>

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
};
// class MyFrame : public wxFrame {
// public:
//   MyFrame();

// private:
//   void OnHello(wxCommandEvent &event);
//   void OnExit(wxCommandEvent &event);
//   void OnAbout(wxCommandEvent &event);
// };
enum { ID_Login_Frame = 1, ID_Matchup_Frame };
wxIMPLEMENT_APP(FantasyApp);
bool FantasyApp::OnInit() {
  // Initialize needed stuff.
  wxInitAllImageHandlers();
  account_manager_ = new fantasy_ball::AccountManager("FantasyApp");
  fantasy_client_ = new fantasy_ball::FantasyServiceClient(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  // Create all the UI frames.
  auto *login_frame = new LoginFrame(this->GetTopWindow(), ID_Login_Frame,
                                     wxT("Fantasy Ball League"));
  auto *matchup_frame = new MatchupFrame(this->GetTopWindow(), ID_Matchup_Frame,
                                         wxT("Fantasy Ball League"));

  // Determine which initial page to show.
  if (account_manager_->HasSession()) {
    matchup_frame->Show(true);
  } else {
    login_frame->Show(true);
  }
  this->GetTopWindow()->Centre();
  return true;
}

int FantasyApp::OnExit() {
  delete account_manager_;
  delete fantasy_client_;
  return 0;
}