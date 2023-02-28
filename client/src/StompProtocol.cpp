#include <stdlib.h>
#include "../include/StompProtocol.h"
#include <sstream>
using std::string;


//constructor
StompProtocol::StompProtocol() : connected(false), counter(0), gameToId(), idToGame(), gameUserMap(), reciptIdMap(), userName(""){}
StompProtocol::StompProtocol(const StompProtocol &other) : handler(other.getConnectionHandler()), connected(other.isConnected()), 
                                                            counter(other.getCounter()), gameToId(other.getGameToId()), idToGame(other.getIdToGame()),
                                                                    gameUserMap(other.getGameUserMap()), reciptIdMap(other.getReceiptIdMap()), userName(other.getUserName()){}


//main process
bool StompProtocol::processInputKeyboard(string msg){
    std::vector<string> request = getRequest(msg);
    string command = getCommand(request);
    bool validCommand = isTheCommandValid(command);

    if(validCommand && command != "summary" && command != "report"){
        Frame frameToSend = Frame(*this, request);
        string toSend = frameToSend.createTheFrame(command, request);
        
        return processTheRightCommand(request, toSend);

    }
    else if(validCommand && command == "summary"){
        processSummary(request);
    }
    else if(validCommand && command == "report"){
        bool path = checkValidPath(request.at(1));
        if(path)
            processReport(request.at(1));
    }

    return false;
    
}

bool StompProtocol::processInputServer(string msg){
    vector<string> message = split(msg, "\n\n");
    vector<string> headers = split(message[0], "\n");

    if(message.size() == 2){  // there is a body to the message.
        string body = message[1];
        if(headers[0] == "CONNECTED")
            return processConnected(headers); // should return false.
        else if(headers[0] == "MESSAGE")
            return processMessage(headers, body);  // should return false.
        else if(headers[0] == "RECEIPT")
            return processReceipt(headers); // should return false unless if disconnected receipt came.
        else if(headers[0] == "ERROR")
            return processError(headers, body);  // should return true.
    }
    else{
        if(headers[0] == "CONNECTED")
            return processConnected(headers);
        else if(headers[0] == "RECEIPT")
            return processReceipt(headers);

    }
    return false;

}

//more functions
void StompProtocol::promoteCounter(){
    counter++;
}

bool StompProtocol::isTheCommandValid(string command){
    if(command != "login" && command != "join" && command != "exit" && command != "report" && command != "summary" && command != "logout"){
        std::cout << "The first word does not match any command" << std::endl;
        return false;
    }
    return true;
}

bool StompProtocol::processTheRightCommand(vector<string> request, string toSend){
    if(getCommand(request) == "login")
        return processLogin(request, toSend);

    if(isConnected()){
        if(getCommand(request) == "join")
            processJoin(request, toSend);

        if(getCommand(request) == "exit")
            processExit(request, toSend);

        if(getCommand(request) == "logout")
            processLogout(request, toSend);
        
    }
    else
        std::cout << "The user is not connected, can not accept the command. You should log in first." << std::endl;
    
    return false;
}

void StompProtocol::sendFrame(string toSendToTheServer) {
    handler->sendFrameAscii(toSendToTheServer, '\0');
}

vector<string> StompProtocol::split(string msg, string delimiter){
    vector<string> ans = vector<string>();

    while(msg.find(delimiter) != string::npos){
        int i = msg.find(delimiter);
        string msg2 = msg.substr(0, i);
        msg = msg.substr(i + 1, msg.size() - i - 1);
        ans.push_back(msg2);
    }

    if(!msg.empty())
        ans.push_back(msg);

    return ans;
}

bool StompProtocol::checkValidPath(string path){
    string delimiter = ".";
    std::vector<string> type = split(path, delimiter);

    if(type.at(1) != "json"){
        std::cout << "No json file was found" << std::endl;
        return false;
    }
    return true;

}

void StompProtocol::addSubscription(int id, string gameName){
    idToGame.emplace(id, gameName);
    gameToId.emplace(gameName, id);
}

void StompProtocol::addRecieptId(int receipt){
    reciptIdMap.emplace(receipt, "disconnect");

}


//getters
string StompProtocol::getCommand(std::vector<string> request){
    return request.at(0);

}

int StompProtocol::getCounter() const{
    return counter;
}

std::vector<string> StompProtocol::getRequest(string msg){
    string delimiter = " ";
    std::vector<string> output = split(msg, delimiter);
    
    return output;    
}

int StompProtocol::getSubIdFromGameName(string gameName) const{
    return gameToId.at(gameName);
}

ConnectionHandler* StompProtocol::getConnectionHandler() const{
    return handler;
}

bool StompProtocol::isConnected() const{
    return connected;
}
map<string, int> StompProtocol::getGameToId() const{
    return gameToId;
}
map<int, string> StompProtocol::getIdToGame() const{
    return idToGame;
}
map<string, map<string, Game>> StompProtocol::getGameUserMap() const{
    return gameUserMap;
}
string StompProtocol::getUserName() const{
    return userName;
}


//specific process from keyboard

bool StompProtocol::processLogin(vector<string> request, string toSend){
    string user = request[2];
    string password = request[3];
    vector<string> spl = split(request[1], ":");
    string host = spl[0];
    short port = stoi(spl[1]);


    userName = user;
    handler = new ConnectionHandler(host, port); //  wrtie operator=

    if (!handler->connect()) {
        std::cout << "Cannot connect to " << host << ":" << port << std::endl;
        return false;
    }
    sendFrame(toSend);
    return true;
}

void StompProtocol::processJoin(vector<string> request, string toSend){
    sendFrame(toSend);
    string gameName = request.at(1);
    
    std::cout << "Joined channel " + gameName << std::endl;
    
}

void StompProtocol::processExit(vector<string> request, string toSend){
    sendFrame(toSend);
    string gameName = request.at(1);
    
    int id = gameToId[gameName];

    gameToId.erase(gameName);
    idToGame.erase(id);

    std::cout << "Exited channel " + gameName << std::endl;
    
}

void StompProtocol::processLogout(vector<string> request, string toSend){    
    sendFrame(toSend);
    //map the reciept
    connected = false;  
    
}

void StompProtocol::processReport(string path){
    names_and_events report = parseEventsFile(path);
    string destination = "" + report.team_a_name + "_" + report.team_b_name;
    std::vector<Event> events = report.events;

    for(Event toSend: events){
        string body = toSend.eventToString(userName);
        string FrameToSend = createSENDframe(destination, body);
        sendFrame(FrameToSend);
    }
    
}

void StompProtocol::processSummary(vector<string> request){
    if(request.size() == 4){
        string gameName = request[1];
        string userName = request[2];
        string fileName = request[3];
        
        if(gameUserMap.find(gameName) != gameUserMap.end()){ // chech is the game exist.
            if(gameUserMap[gameName].find(userName) != gameUserMap[gameName].end()){  // chech if the userName exist
                gameUserMap[gameName][userName].summaraize(fileName);  // summarize.
            }

        }
    } 
}

bool StompProtocol::processConnected(vector<string> headers){
    connected = true;
    std::cout << "Login successful" << std::endl;
    return false;
}
bool StompProtocol::processMessage(vector<string> headers, string body){
    string gameName = split(headers[3], ":")[1];

    vector<string> bodyHeaders = split(body, "\n");
    string userName = split(bodyHeaders[0], ":")[1];
    string teamAName = split(bodyHeaders[1], ":")[1];
    string teamBName = split(bodyHeaders[2], ":")[1];
    string eventName = split(bodyHeaders[3], ":")[1];
    string time = split(bodyHeaders[4], ":")[1];

    map<string, string> gameUpdates =  getGameUpdates(bodyHeaders);
    map<string, string> teamAUpdates =  getTeamAUpdates(bodyHeaders);
    map<string, string> teamBUpdates =  getTeamBUpdates(bodyHeaders);

    int i = find("description:", bodyHeaders);
    string description = bodyHeaders[i + 1];
    Event event = Event(eventName, teamAName, teamBName, stoi(time), gameUpdates, teamAUpdates, teamBUpdates, description);

    if(gameUserMap.find(gameName) != gameUserMap.end()){  // there is allready game like this.
        if(gameUserMap[gameName].find(userName) != gameUserMap[gameName].end()){  // the user is allready report something about this game.
            gameUserMap[gameName][userName].addAnEvent(event);
        }
        else{  // the user dont report nothing on this event.
            vector<Pair> generalGameUpadates = getPairGameUpdates(bodyHeaders);
            vector<Pair> teamAUpadates = getPairTeamAUpdates(bodyHeaders);
            vector<Pair> teamBUpadates = getPairTeamBUpdates(bodyHeaders);

            Game game = Game(gameName, teamAName, teamBName, generalGameUpadates, teamAUpadates, teamBUpadates);
            game.addAnEvent(event);
            gameUserMap[gameName].emplace(userName, game);
        }
    }
    else{ // game does not exist
        vector<Pair> generalGameUpadates = getPairGameUpdates(bodyHeaders);
        vector<Pair> teamAUpadates = getPairTeamAUpdates(bodyHeaders);
        vector<Pair> teamBUpadates = getPairTeamBUpdates(bodyHeaders);
        Game game = Game(gameName, teamAName, teamBName, generalGameUpadates, teamAUpadates, teamBUpadates);
        game.addAnEvent(event);
        gameUserMap.emplace(gameName, map<string, Game>());
        gameUserMap[gameName].emplace(userName, game);
    }
    
    return false;
}
bool StompProtocol::processReceipt(vector<string> headers){
    bool ans = false;
    string reciptId = "";

    for(string str : headers){  // find the recipt
        vector<string> spl = split(str, ":");
        if(spl[0] == "receipt-id"){
            reciptId = spl[1];
            break;
        }
    }

    if(reciptId != "" && reciptIdMap.find(stoi(reciptId)) != reciptIdMap.end()){
        if(reciptIdMap[stoi(reciptId)] == "disconnect"){
            ans = true;
            connected = false;
            clearAllData();
        }
    }


    return ans;
}
bool StompProtocol::processError(vector<string> headers, string body){
    connected = false;
    for(string str : headers){
        vector<string> spl = split(str, ":");
        if(spl[0] == "message"){
            string errorMessage = spl[1];
            std::cout << errorMessage << std::endl;
            break;
        }
    }
    return true;
}
map<string, string> StompProtocol::getGameUpdates(vector<string> bodyHeaders){
    map<string, string> ans = map<string, string>();
    int i = find("general game updates:", bodyHeaders);
    int j = find("team a updates:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.emplace(event[0], event[1]);
        }
    }
    return ans;
}

map<string, string> StompProtocol::getTeamAUpdates(vector<string> bodyHeaders){
    map<string, string> ans = map<string, string>();
    int i = find("team a updates:", bodyHeaders);
    int j = find("team b updates:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.emplace(event[0], event[1]);
        }
    }
    return ans;
}
map<string, string> StompProtocol::getTeamBUpdates(vector<string> bodyHeaders){
    map<string, string> ans = map<string, string>();
    int i = find("team b updates:", bodyHeaders);
    int j = find("description:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.emplace(event[0], event[1]);
        }
    }
    return ans;   
}
vector<Pair> StompProtocol::getPairGameUpdates(vector<string> bodyHeaders){
    vector<Pair> ans = vector<Pair>();
    int i = find("general game updates:", bodyHeaders);
    int j = find("team a updates:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.push_back(Pair(event[0], event[1]));
        }
    }
    return ans;
}
vector<Pair> StompProtocol::getPairTeamAUpdates(vector<string> bodyHeaders){
    vector<Pair> ans = vector<Pair>();
    int i = find("team a updates:", bodyHeaders);
    int j = find("team b updates:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.push_back(Pair(event[0], event[1]));
        }
    }
    return ans;
}
vector<Pair> StompProtocol::getPairTeamBUpdates(vector<string> bodyHeaders){
    vector<Pair> ans = vector<Pair>();
    int i = find("team b updates:", bodyHeaders);
    int j = find("description:", bodyHeaders);

    if(i >=0 && j >=0){
        for(int index = i; index < j; index++){
            vector<string> event = split(bodyHeaders[index], ":");
            ans.push_back(Pair(event[0], event[1]));
        }
    }
    return ans;
}

int StompProtocol::find(string str, vector<string> vector){
    for(size_t i = 0; i < vector.size(); i++){
        if(vector[i] == str)
            return i;
    }
    return -1;
}

//frames
string StompProtocol::createSENDframe(string destination, string body) {
    string toSend = "SEND\ndestination:" + destination + "\n\n" + body;
	return toSend;
}
map<int, string> StompProtocol::getReceiptIdMap() const {
    return reciptIdMap;
}
void StompProtocol::clearAllData(){
    gameUserMap.clear();
    reciptIdMap.clear();
    gameToId.clear();
    idToGame.clear();
    counter = 0;
    connected = false;
}









