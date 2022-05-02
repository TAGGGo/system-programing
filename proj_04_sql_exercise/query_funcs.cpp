#include "query_funcs.h"
#include <iomanip>
void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    work work(*C);
    stringstream cmd;
    cmd << "INSERT INTO player (team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg)\n"
        << "VALUES (" << team_id << ", " << jersey_num << ", " << work.quote(first_name) << ", " << work.quote(last_name) << ", "
        << mpg << ", " << ppg << ", " << rpg << ", " << apg << ", " << spg << ", " << bpg << "); ";
    //cout << cmd.str() << endl;
    work.exec(cmd);
    work.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    work work(*C);
    stringstream cmd;
    cmd << "INSERT INTO team (name, state_id, color_id, wins, losses)\n"
        << "VALUES (" << work.quote(name) << ", " << state_id << ", " << color_id << ", " << wins << ", " << losses << "); ";
    work.exec(cmd);
    work.commit();
}


void add_state(connection *C, string name)
{
    work work(*C);
    stringstream cmd;
    cmd << "INSERT INTO state (name)\n"
        << "VALUES (" << work.quote(name) << "); ";
    work.exec(cmd);
    work.commit();
}


void add_color(connection *C, string name)
{
    work work(*C);
    stringstream cmd;
    cmd << "INSERT INTO color (name)\n"
        << "VALUES (" << work.quote(name) << "); ";
    work.exec(cmd);
    work.commit();
}


void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
    vector<string> label = {"mpg", "ppg", "rpg", "apg", "spg", "bpg"};
    vector<int> val = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
    vector<pair<double, double> > range = {{min_mpg, max_mpg}, {min_ppg, max_ppg}, {min_rpg, max_rpg}, {min_apg, max_apg}, {min_spg, max_spg}, {min_bpg, max_bpg}};
    stringstream cmd;
    bool begin = true;
    cmd << "SELECT * FROM player";
    for(int i = 0; i < label.size(); ++i) {
        if(val[i]) {
            if(begin) {
                cmd << " WHERE ";
            }
            else {
                cmd << " AND ";
            }
            cmd << "(" << label[i] << " BETWEEN " << fixed << setprecision(1) << range[i].first << " AND " << range[i].second << ")";
            begin = false;
        }
    }
    cmd << ";";
    nontransaction non_work(*C);
    result rel(non_work.exec(cmd.str()));
    //cout << cmd.str() << endl;
    // showing the output
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG\n";
    for (auto it = rel.begin(); it != rel.end(); ++it) {
        cout << it[0].as<int>() << " " << it[1].as<int>() << " " << it[2].as<int>()
            << " " << it[3].as<string>() << " " << it[4].as<string>() << " "
            << it[5].as<int>() << " " << it[6].as<int>() << " " << it[7].as<int>()
            << " " << it[8].as<int>() << " " << fixed << setprecision(1)
            << it[9].as<double>() << " " << it[10].as<double>() << "\n";
    }
}


void query2(connection *C, string team_color)
{
    work work(*C);
    string cmd = "SELECT team.name FROM TEAM, COLOR WHERE team.color_id = color.color_id and color.name = " + work.quote(team_color) + ";";
    work.commit();
    nontransaction non_work(*C);
    result rel(non_work.exec(cmd));
    cout << "NAME\n";
    for(auto it = rel.begin(); it != rel.end(); ++it) {
        cout << it[0].as<string>() << "\n";
    }
}


void query3(connection *C, string team_name)
{
    work work(*C);
    string cmd = "SELECT player.first_name, player.last_name FROM team, player WHERE team.team_id = player.team_id and team.name = " + work.quote(team_name) + " ORDER BY ppg desc;";
    work.commit();
    nontransaction non_work(*C);
    result rel(non_work.exec(cmd));
    cout << "FIRST_NAME LAST_NAME\n";
    for(auto it = rel.begin(); it != rel.end(); ++it) {
        cout << it[0].as<string>() << " " << it[1].as<string>() << "\n";
    }
}


void query4(connection *C, string team_state, string team_color)
{
    work work(*C);
    string cmd = "SELECT player.uniform_num, player.first_name, player.last_name FROM team, player, color, state WHERE team.state_id = state.state_id AND team.color_id = color.color_id AND team.team_id = player.team_id AND state.name = " + work.quote(team_state) + " AND color.name = " + work.quote(team_color) + ";";
    work.commit();
    nontransaction non_work(*C);
    result rel(non_work.exec(cmd));
    cout << "UNIFORM_NUM FIRST_NAME LAST_NAME\n";
    for(auto it = rel.begin(); it != rel.end(); ++it) {
        cout << it[0].as<int>() << " " << it[1].as<string>() << " " << it[2].as<string>() << "\n";
    }
}


void query5(connection *C, int num_wins)
{
    work work(*C);
    stringstream cmd;
    cmd << "SELECT player.first_name, player.last_name, team.name, team.wins FROM team, player WHERE team.team_id = player.team_id AND team.wins > " << num_wins << ";";
    work.commit();
    nontransaction non_work(*C);
    result rel(non_work.exec(cmd));
    cout << "FIRST_NAME LAST_NAME NAME WINS\n";
    for(auto it = rel.begin(); it != rel.end(); ++it) {
        cout << it[0].as<string>() << " " << it[1].as<string>() << " " << it[2].as<string>() << " " << it[3].as<int>() << "\n";
    }
}
