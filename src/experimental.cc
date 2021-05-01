// wxWidgets "Hello World" Program

#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <wx/datetime.h>

#include <fmt/core.h>
#include <pqxx/pqxx>

#include "league_fetcher.h"
#include "postgre_sql_fetch.h"
#include "util.h"

// For compilers that support precompilation, includes "wx/wx.h".
// #include <wx/wxprec.h>
// #ifndef WX_PRECOMP
// #include <wx/wx.h>
// #endif
// class MyApp : public wxApp
// {
// public:
//     virtual bool OnInit();
// };
// class MyFrame : public wxFrame
// {
// public:
//     MyFrame();
// private:
//     void OnHello(wxCommandEvent& event);
//     void OnExit(wxCommandEvent& event);
//     void OnAbout(wxCommandEvent& event);
// };
// enum
// {
//     ID_Hello = 1
// };
// wxIMPLEMENT_APP(MyApp);
// bool MyApp::OnInit()
// {
//     MyFrame *frame = new MyFrame();
//     frame->Show(true);
//     return true;
// }
// MyFrame::MyFrame()
//     : wxFrame(NULL, wxID_ANY, "Hello World")
// {
//     wxMenu *menuFile = new wxMenu;
//     menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
//                      "Help string shown in status bar for this menu item");
//     menuFile->AppendSeparator();
//     menuFile->Append(wxID_EXIT);
//     wxMenu *menuHelp = new wxMenu;
//     menuHelp->Append(wxID_ABOUT);
//     wxMenuBar *menuBar = new wxMenuBar;
//     menuBar->Append(menuFile, "&File");
//     menuBar->Append(menuHelp, "&Help");
//     SetMenuBar( menuBar );
//     CreateStatusBar();
//     SetStatusText("Welcome to wxWidgets!");
//     Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
//     Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
//     Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
// }
// void MyFrame::OnExit(wxCommandEvent& event)
// {
//     Close(true);
// }
// void MyFrame::OnAbout(wxCommandEvent& event)
// {
//     wxMessageBox("This is a wxWidgets Hello World example",
//                  "About Hello World", wxOK | wxICON_INFORMATION);
// }
// void MyFrame::OnHello(wxCommandEvent& event)
// {
//     wxLogMessage("Hello world from wxWidgets!");
// }

static std::string base64_encode(const std::string &in) {

  std::string out;

  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
              [(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
            [((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

static std::string base64_decode(const std::string &in) {

  std::string out;

  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++)
    T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] =
        i;

  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1)
      break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

CURLcode curl_post(const std::string &url, const std::string &api_key,
                   CURL *curl_instance, std::string *buffer) {
  struct curl_slist *list = NULL;
  const std::string api_token =
      "Basic " + base64_encode(api_key + ":MYSPORTSFEEDS");
  const std::string auth_token = "Authorization: " + api_token;
  list = curl_slist_append(list, auth_token.c_str());
  curl_easy_setopt(curl_instance, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_instance, CURLOPT_WRITEFUNCTION, ::WriteCallback);
  curl_easy_setopt(curl_instance, CURLOPT_WRITEDATA, buffer);
  curl_easy_setopt(curl_instance, CURLOPT_FRESH_CONNECT, 1);
  curl_easy_setopt(curl_instance, CURLOPT_HTTPHEADER, list);
  CURLcode res = curl_easy_perform(curl_instance);
  return res;
}

// Removes fractional seconds and 'Z' characters from a full ISO time format
// string. Needed in order to use the WxDate util functions.
std::string trim_iso_extra(std::string iso_date) {
  // std::string iso_date = "2021-03-20T00:00:00.000Z";
  auto trim_pos = iso_date.rfind(".000Z");
  if (trim_pos == std::string::npos) {
    return "";
  }
  return iso_date.substr(0, trim_pos);
}

wxDateTime iso_to_local_time(std::string iso_date) {

  const std::string iso_date_trim = trim_iso_extra(iso_date);
  wxDateTime date;
  // 2021-03-20T00:00:00.000Z -- need to trim .000Z part.
  if (!date.ParseISOCombined(iso_date_trim)) {
    return date;
  } else {
    // calc the difference between the UTC and the local tz
    wxTimeSpan ts = date.Subtract(date.ToUTC());

    // now offset the datetime by the difference
    wxDateTime date_local = date + ts;
    std::cout << "Local time: " << date_local.FormatISOCombined().c_str()
              << std::endl;
    std::cout << "Default time: " << date.FormatISOCombined().c_str()
              << std::endl;
    return date_local;
  }
}

std::string get_local_date_from_iso(std::string iso_date) {
  auto date = iso_to_local_time(iso_date);
  return date.FormatISOCombined().ToStdString();
}

// int main() {

//   const std::string api_key = fantasy_ball::endpoint::read_msf_api_key();
//   std::string url = "https://scrambled-api.mysportsfeeds.com/v2.1/pull/nba/"
//                     "players.json?player=jordan-poole";
//   //
//   "https://api.mysportsfeeds.com/v2.1/pull/nba/2020-2021-regular/date/20210319/player_gamelogs.json?player=10134"
//   using json = nlohmann::json;
//   json data;

//   std::string buffer;
//   CURL *curl_instance = curl_easy_init();
//   const std::string cache_filename = "cache.json";
//   bool cache_exist = std::filesystem::exists(cache_filename);

//   if (!cache_exist) {
//     auto res = curl_post(url, api_key, curl_instance, &buffer);
//     if (res) {
//       std::cout << "Error occurred making CURL request.\n";
//     }
//     curl_easy_cleanup(curl_instance);

//     std::ofstream file(cache_filename);
//     std::cout << buffer << "\n";
//     data = json::parse(buffer);
//     file << std::setw(4) << data << std::endl;
//   } else {
//     std::ifstream file(cache_filename);
//     file >> data;
//   }

//   std::cout << "packet: " << std::endl;

//   // auto o = json::parse(buffer_);
//   // std::ofstream cache("file.json");
//   // cache << std::setw(4) << o << std::endl;

//   // // explicit conversion to string
//   // //   std::string s = j.dump(); // {"happy":true,"pi":3.141}

//   // // serialization with pretty printing
//   // // pass in the amount of spaces to indent
//   std::cout << data.dump(4) << std::endl;
//   // std::string iso_date = "2021-03-20T00:00:00.000Z";
//   // std::cout << "Local time: " << get_local_date_from_iso(iso_date) <<
//   "\n";
//   // this pretends to be the datetime from the server in the UTC
//   // wxDateTime dtUTC;
//   // // 2021-03-20T00:00:00.000Z -- need to trim .000Z part.
//   // if (!dtUTC.ParseISOCombined(iso_date)) {
//   //   std::cout << "Couldn't parse.\n";
//   // } else {
//   //   // calc the difference between the UTC and the local tz
//   //   wxTimeSpan ts = dtUTC.Subtract(dtUTC.ToUTC());

//   //   // now offset the datetime by the difference
//   //   wxDateTime dtLocal = dtUTC + ts;
//   //   std::cout << "Local time: " << dtLocal.FormatISOCombined().c_str()
//   //             << std::endl;
//   //   std::cout << "Default time: " << dtUTC.FormatISOCombined().c_str()
//   //             << std::endl;
//   // }
//   // // for (json::iterator it = o.begin(); it != o.end(); ++it) {
//   // //   std::cout << it.key() << "\n";
//   // // }

//   // // Here's how to index and get an array from the json object.
//   // const auto &gamelogs = data["gamelogs"];
//   // std::cout << "size: " << gamelogs.size() << "\n";
//   // for (const auto &log : gamelogs) {
//   //   if (!log.contains("player")) {
//   //     std::cout << "No player.\n";
//   //     break;
//   //   }
//   //   std::cout << log["player"]["id"] << "\n";
//   // }

//   // const auto &gamelogs = data.find("gamelogs");
//   // if (gamelogs == data.end()) {
//   //   std::cout << "Found no gamelogs.\n";
//   //   return 0;
//   // }
//   // for (const auto &log : *gamelogs) {
//   //   if (!log.contains("player")) {
//   //     std::cout << "No player.\n";
//   //     break;
//   //   }
//   //   std::cout << log["player"]["id"] << "\n";
//   // }

//   // auto box_score_iter = data["boxscore"].find("players");

//   // // // or via find and an iterator
//   // if (box_score_iter != o.find("boxscore")->end()) {
//   //   std::cout << box_score_iter->dump(4);
//   // } else {
//   //   std::cout << "Could not find boxscore key.\n";
//   // }
//   // {
//   //     "happy": true,
//   //     "pi": 3.141
//   // }
//   return 0;
// }

// int main() {
//   try {
//     const std::string config = fantasy_ball::postgre::read_postgre_config();
//     std::cout << "config: " << config << std::endl;
//     if (config.empty()) {
//       std::cout << "Error reading config file." << std::endl;
//       return 0;
//     }
//     auto C = std::make_unique<pqxx::connection>(config);
//     std::cout << "Connected to " << C->dbname() << std::endl;

//     fantasy_ball::PostgreSQLFetch psql_fetch;
//     auto commands =
//         psql_fetch.read_from_file("postgre_scripts/create_user_account.sql");
//     std::cout << "Commands: " << commands.size() << std::endl;
//     for (const std::string &command : commands) {
//       std::cout << "--->" << command << std::endl << std::endl;
//     }

//     // Write to database.
//     {
//       pqxx::work W{*C};
//       for (const std::string &command : commands) {
//         W.exec0(command);
//       }

//       std::cout << "Making changes definite.\n";
//       pqxx::result R{W.exec("SELECT * FROM user_account")};

//       std::cout << "Found " << R.size() << " user_account:\n";
//       for (auto row : R) {
//         for (auto const &field : row) {
//           std::cout << field.c_str() << '\t';
//         }
//         std::cout << std::endl;
//       }
//       W.commit();
//     }

//     // Read from database.
//     // {
//     //   pqxx::work W{*C};

//     //   pqxx::result R{W.exec("SELECT * FROM class")};

//     //   std::cout << "Found " << R.size() << " classes:\n";
//     //   for (auto row : R) {
//     //     for (auto const &field : row) {
//     //       std::cout << field.c_str() << '\t';
//     //     }
//     //     std::cout << std::endl;
//     //   }

//     //   // std::cout << "Doubling all employees' salaries...\n";
//     //   // W.exec0("UPDATE employee SET salary = salary*2");

//     //   std::cout << "Making changes definite." << std::endl;
//     //   W.commit();
//     // }

//   } catch (std::exception const &e) {
//     std::cerr << e.what() << 'a';
//     return 1;
//   }
//   return 0;
// }

int main() {
  // Create the psql_fetcher.
  fantasy_ball::PostgreSQLFetch psql_fetch;
  bool delete_tables = true;
  bool db_init_success = psql_fetch.Init();
  if (!db_init_success) {
    std::cout << "Couldn't initialize PostgreSQL database." << std::endl;
    return 0;
  }
  auto *connection = psql_fetch.GetCurrentConnection();
  pqxx::work W{*connection};
  if (delete_tables) {
    psql_fetch.DeleteBaseTables(&W);
  }
  psql_fetch.CreateBaseTables(&W);
  W.commit();
  std::cout << "Completed psql_fetch initialization." << std::endl;

  // Create the league fetcher.
  fantasy_ball::LeagueFetcher league_fetcher(&psql_fetch);
  std::string auth_token = league_fetcher.CreateUserAccount(
      "me", "me@me.com", "password", "me", "em");
  std::cout << "Added account." << std::endl;
  int league_id = league_fetcher.CreateLeague(auth_token, "CISC. 4900");
  std::cout << fmt::format("Created league with id {}.", league_id)
            << std::endl;
  auto basic_information = league_fetcher.GetBasicUserInformation(auth_token);
  std::cout << fmt::format(
                   "username: {}, email: {}, first_name: {}, last_name: {}.",
                   std::get<0>(basic_information),
                   std::get<1>(basic_information),
                   std::get<2>(basic_information),
                   std::get<3>(basic_information))
            << std::endl;
  league_fetcher.MakeDraftPick(auth_token, 1, 1, league_id, "1,2");
  std::cout << "Added draft pick." << std::endl;
  return 0;
}