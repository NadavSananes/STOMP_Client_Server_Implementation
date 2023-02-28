#pragma once
#include "ConnectionHandler.h"
#include <string>
#include <vector>
#include <map>
#include "Game.h"
#include "event.h"
using std::map;

#ifndef __COMMON_H__
#define __COMMON_H__
#include "Frame.h"
#endif


// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    ConnectionHandler *handler;
    bool connected;
    int counter;
    map<string, int> gameToId;
    map<int, string> idToGame;
    map<string, map<string, Game>> gameUserMap;  // map bitween the game name to user map of games.
    map<int, string> reciptIdMap;
    string userName;

//string of path/ file file;



public:
    //constructors
    StompProtocol();
    StompProtocol(const StompProtocol &other);
    
    //main process
    bool processInputKeyboard(string msg);
    bool processInputServer(string msg);

    //more functions
    void promoteCounter();
    bool isTheCommandValid(string command);
    bool processTheRightCommand(vector<string> request, string teSend);
    void sendFrame(string toSendToTheServer);
    vector<string> split(string msg, string delimiter);
    bool checkValidPath(string path);
    void clearAllData();
    void addSubscription(int id, string gameName);
    void addRecieptId(int receipt);


    //specific process from keyboard
    bool processLogin(vector<string> request, string toSend);
    void processJoin(vector<string> request, string toSend);
    void processExit(vector<string> request, string toSend);
    void processLogout(vector<string> request, string toSend);
    void processSummary(std::vector<string> request);
    void processReport(string path);

    //specific process from server
    bool processConnected(vector<string> headers);
    bool processMessage(vector<string> headers, string body);
    bool processReceipt(vector<string> headers);
    bool processError(vector<string> headers, string body);


    //getters
    int getCounter() const;
    std::vector<string> getRequest(string msg);
    ConnectionHandler* getConnectionHandler() const;
    bool isConnected() const;
    map<string, int> getGameToId() const;
    map<int, string> getIdToGame() const;
    map<string, map<string, Game>> getGameUserMap() const;
    int find(string str, vector<string> vector);
    map<string, string> getTeamBUpdates(vector<string> bodyHeaders);
    map<string, string> getTeamAUpdates(vector<string> bodyHeaders);
    map<string, string> getGameUpdates(vector<string> bodyHeaders);
    map<int, string> getReceiptIdMap() const;
    string getUserName() const;
    string getCommand(std::vector<string> request);
    int getSubIdFromGameName(string gameName) const;

    vector<Pair> getPairGameUpdates(vector<string> bodyHeaders);
    vector<Pair> getPairTeamAUpdates(vector<string> bodyHeaders);
    vector<Pair> getPairTeamBUpdates(vector<string> bodyHeaders);

    //frames
    string createSENDframe(string destination, string body);

};
