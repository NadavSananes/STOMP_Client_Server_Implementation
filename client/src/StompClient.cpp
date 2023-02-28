#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <thread>
#include <iostream>

class Task{
	private:
		ConnectionHandler *handler;
		StompProtocol *protocol;

	public:
		Task(ConnectionHandler *handler, StompProtocol *protocol): handler(handler), protocol(protocol){};

		void operator()(){
			while(1){

        		std::string answer;
        		if (!handler->getFrameAscii(answer, '\0')) {
            		std::cout << "Disconnected. Exiting...\n" << std::endl;
            		break;
        		}
				if(protocol->processInputServer(answer)){
					handler->close();
					std::cout << "disconnect.." << std::endl;
					break;
				}  
				
			}
		}
};

int main(int argc, char *argv[]) {
	StompProtocol protocol = StompProtocol();
	Task* task;
	std::thread *th;
	while (1) {
			const short bufsize = 1024;
			char buf[bufsize];
			std::cin.getline(buf, bufsize);
			std::string line(buf);
			if(protocol.processInputKeyboard(line)){
				task = new Task(protocol.getConnectionHandler(), &protocol); // create the other thread task.
				th = new std::thread(std::ref(*task));  // run the task.
			}
	}
	return 0;
}


