#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include <sstream>
#include <string>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

//Establish a connection to the database
//Parameters: database name, user name, user password
connection * ConnectDatabase() {
  connection * ret = 
    new connection("dbname=ACC_BBALL user=postgres password=passw0rd hostaddr=127.0.0.1 port=5432");
  return ret;
}

void createTables(string file, connection * C) {
  string cmd, tmp;
  ifstream ss;
  ss.open(file.c_str(), ifstream::in);
  while(getline(ss, tmp)) {
    cmd += tmp;
  }
  work work(*C);
  work.exec(cmd);
  work.commit();
  ss.close();
}

void readPlayer(string file, connection * C) {
  string tmp, first_name, last_name;
  int player_id, team_id, uniform_num, mpg, ppg, rpg, apg;
  double spg, bpg;
  ifstream ss;
  ss.open(file.c_str(), ifstream::in);
  while(getline(ss, tmp)) {
    stringstream sss(tmp);
    sss >> player_id >> team_id >>  uniform_num >> first_name >> last_name >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
    add_player(C, team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  ss.close();
}

void readTeam(string file, connection * C) {
  string name, tmp;
  int team_id, state_id, color_id, wins, losses;
  ifstream ss;
  ss.open(file.c_str(), ifstream::in);
  while(getline(ss, tmp)) {
    stringstream sss(tmp);
    sss >> team_id >> name >> state_id >> color_id >> wins >> losses;
    add_team(C, name, state_id, color_id, wins, losses);
  }
  ss.close();
}

void readColor(string file, connection * C) {
  string name, tmp;
  int color_id;
  ifstream ss;
  ss.open(file.c_str(), ifstream::in);
  while(getline(ss, tmp)) {
    stringstream sss(tmp);
    sss >> color_id >> name;
    add_color(C, name);
  }
  ss.close();
}

void readState(string file, connection * C) {
  string name, tmp;
  int state_id;
  ifstream ss;
  ss.open(file.c_str(), ifstream::in);
  while(getline(ss, tmp)) {
    stringstream sss(tmp);
    sss >> state_id >> name;
    add_state(C, name);
  }
  ss.close();
}

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection * C;

  try{
    //Initialize Database
    C = ConnectDatabase();
    if (C->is_open()) {
      //cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }

    //TODO 1: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
    createTables("create.sql", C);

    //TODO 2: load each table with rows from the provided source txt files
    readState("state.txt", C);
    readColor("color.txt", C);
    readTeam("team.txt", C);
    readPlayer("player.txt", C);

    exercise(C);


    //Close database connection
    C->disconnect();
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}


