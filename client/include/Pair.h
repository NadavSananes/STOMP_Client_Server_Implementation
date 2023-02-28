#pragma once
#include <string>
#include <iostream>
#include <vector>
using std::string;

class Pair
{
private:
    string first;
    string second;
    
public:
    Pair(string first, string second);
    string getFirst();
    string getSecond();
};

