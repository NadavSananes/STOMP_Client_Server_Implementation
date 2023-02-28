package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {
    

    public static void main(String[] args) {

        if(args[1].equals("tpc")){
            Server.threadPerClient(Integer.parseInt(args[0]),
                                     ()-> {return new StompMessagingProtocolImpl();},
                                      ()-> {return new StompMessageEncoderDecoderImpl();}).serve();
        }
        else{
            if(args[1].equals("reactor")){  // i gave him 10 threads. how much is needed?
                Server.reactor(10, Integer.parseInt(args[0]),
                                         ()-> {return new StompMessagingProtocolImpl();},
                                          ()-> {return new StompMessageEncoderDecoderImpl();}).serve();
            }
        }
    }
}
