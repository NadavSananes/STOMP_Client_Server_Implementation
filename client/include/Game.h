#pragma once
#include "Pair.h"
#include "event.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using std::string;
using std::vector;
using namespace std;

class Game{

private:
    string gameName;
    string team_a_name;
    string team_b_name;
    vector<Pair> generalGameUpadates;
    vector<Pair> teamAUpadates;
    vector<Pair> teamBUpadates;
    vector<Event> events;

public:
    Game(string gameName, string team_a_name, string team_b_name);
    Game(string gameName, string team_a_name, string team_b_name, vector<Pair> generalGameUpadates, vector<Pair> teamAUpadates, vector<Pair> teamBUpadates);
    Game();
    void summaraize(string file);
    string makeSummaraize();
    void addAnEvent(Event event);

};