#include "Game.h"

Game::Game(string gameName, string team_a_name, string team_b_name) : gameName(gameName), team_a_name(team_a_name),
                                                                        team_b_name(team_b_name), generalGameUpadates(), teamAUpadates(), teamBUpadates(), events(){}
Game::Game() : gameName(""), team_a_name(""), team_b_name(""), generalGameUpadates(), teamAUpadates(), teamBUpadates(), events(){}

Game::Game(string gameName, string team_a_name, string team_b_name, vector<Pair> generalGameUpadates, vector<Pair> teamAUpadates, vector<Pair> teamBUpadates) : gameName(gameName), team_a_name(team_a_name), team_b_name(team_b_name),
generalGameUpadates(generalGameUpadates), teamAUpadates(teamAUpadates), teamBUpadates(teamBUpadates), events(){}


void Game::summaraize(string file){
    ofstream MyFile(file);  // create and open.
    string toSend = makeSummaraize();  // get the message.
    MyFile << toSend;  // write to the file.
    MyFile.close();  // close the file.
}

string Game::makeSummaraize(){
    string ans = "";
    ans += team_a_name +" vs " + team_b_name + "\n";  // first line.

    ans += "Game stats:\n";   // all general stats.
    for(Pair pair : generalGameUpadates){
        ans += pair.getFirst() + ":" + pair.getSecond() + "\n";
    }

    ans += team_a_name + "status:\n";  // all team a stats.
    for(Pair pair : teamAUpadates){
        ans += pair.getFirst() + ":" + pair.getSecond() + "\n";
    }

    ans += team_b_name + "status:\n";  // all team b stats.
    for(Pair pair : teamBUpadates){
        ans += pair.getFirst() + ":" + pair.getSecond() + "\n";
    }

    ans += "Game event reports:\n";
    for(Event event : events){
        ans += event.summarize();
    }

    return ans;
}

void Game::addAnEvent(Event event){
    events.push_back(event);
}