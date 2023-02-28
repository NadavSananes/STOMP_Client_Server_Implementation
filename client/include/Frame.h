#pragma once
#include <stdlib.h>
#include "StompProtocol.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

class StompProtocol;

class Frame
{
private:
    StompProtocol& protocol;
    std::vector<string> request;

    
    

    string createCONNECTframe(string acceptVersion, string host, string login, string passcode);
    string createSUBSCRIBEframe(string destination, string id);
    string createUNSUBSCRIBEframe(string id);
    string createDISCONNECTframe(string receipt);

public:
    //constructor
    Frame(StompProtocol& protocol, std::vector<string> request);

    //some functions
    string createTheFrame(string command, std::vector<string> request);
    vector<string> split(string msg, string delimiter);
    
    //getters
    string getCommand();    

};