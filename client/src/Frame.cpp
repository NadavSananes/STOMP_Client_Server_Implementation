#include "../include/Frame.h"


//constructor
Frame::Frame(StompProtocol& protocol, std::vector<string> request): protocol(protocol),request(request){}


//some functions
string Frame::createTheFrame(string command, std::vector<string> request){
    if(command == "login"){
        string acceptVersion = "1.2";
        string hostPort = request.at(1);
        string username = request.at(2);
        string password = request.at(3);

        std::vector<string> hostAndPort = split(hostPort, ":");
        // string host = hostAndPort.at(0);

        return createCONNECTframe(acceptVersion, "stomp.cs.bgu.ac.il", username, password);
    }

    if(command == "join"){
        string id = std::to_string((protocol).getCounter());
        (protocol).promoteCounter();

        string gameName = request.at(1);

        protocol.addSubscription(stoi(id), gameName);

        return createSUBSCRIBEframe(gameName, id);
    }

    if(command == "exit"){
        string gameName = request.at(1);
        int id = (protocol).getSubIdFromGameName(gameName);

        return createUNSUBSCRIBEframe(std::to_string(id));
    }

    if(command == "logout"){
        int receipt = (protocol).getCounter();
        (protocol).promoteCounter();
        protocol.addRecieptId(receipt);
        
        return createDISCONNECTframe(std::to_string(receipt));
    }        
    return "didnt enter any if";
}

vector<string> Frame::split(string msg, string delimiter){
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




//create specific frame
string Frame::createCONNECTframe(string acceptVersion, string host, string login, string passcode) {
    string frame = "CONNECT\n";
    frame = frame + "accept-version:" + acceptVersion + "\n" + "host:" + host + "\n" + "login:" + login + "\n" + "passcode:" + passcode + "\n\n" ;
	return frame;
}


string Frame::createSUBSCRIBEframe(string destination, string id) {
    string frame = "SUBSCRIBE\n";
    frame = frame + "destination:" + destination + "\n" + "id:" + id + "\n\n";
	return frame;
}

string Frame::createUNSUBSCRIBEframe(string id) {
    string frame = "UNSUBSCRIBE\n";
    frame = frame + "id:" + id + "\n\n";
	return frame;
}

string Frame::createDISCONNECTframe(string receipt) {
    string frame = "DISCONNECT\n";
    frame = frame + "receipt:" + receipt + "\n\n";
	return frame;
}


//getters
string Frame::getCommand(){
    return request.at(0);
}