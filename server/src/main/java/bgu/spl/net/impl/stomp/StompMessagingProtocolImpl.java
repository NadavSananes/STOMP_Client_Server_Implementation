package bgu.spl.net.impl.stomp;

import java.io.IOException;
import java.util.List;
import java.util.logging.Handler;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.NonBlockingConnectionHandler;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> , MessagingProtocol<String>{ //should be T?

    /**
     * The class field.
     */
    private ConnectionsImpl<String>  connections;
    private int connectionId;
    private ConnectionHandler<String> connectionHandler;
    private int messageIdCounter = 1;
    boolean shouldTerminate = false;

    @Override
    public void start(int connectionId, Connections<String> connections, ConnectionHandler<String> handler) {
        this.connections = (ConnectionsImpl<String>)connections;
        this.connectionId = connectionId;
        this.connectionHandler = handler;
        
    }

    @Override
    public void process(String message) {
        System.out.println(message);
        Frame  msgFrame = new Frame(message);
        
        if(msgFrame.getCommand().equals("CONNECT")){
            processConnect(msgFrame);
        }
        else if(msgFrame.getCommand().equals("DISCONNECT"))
            processDisconnect(msgFrame);
        
        else if(msgFrame.getCommand().equals("SUBSCRIBE"))
            processSubscribe(msgFrame);

        else if(msgFrame.getCommand().equals("UNSUBSCRIBE"))
            processUnsubscribe(msgFrame);

        else if(msgFrame.getCommand().equals("SEND"))
            processSend(msgFrame);
        else{  // there is no such command.
            String errorFrame = createErrorFrame("WrongCommand", msgFrame.getHeader("receipt"), "There is no such command", null);
            sendError(errorFrame);
            closeConnection();
        }

    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void processConnect(Frame frameToProcess){
        String userName = frameToProcess.getHeader("login");
        String acceptVersion = frameToProcess.getHeader("accept-version");
        String host = frameToProcess.getHeader("host");
        String passcode = frameToProcess.getHeader("passcode");
        String receipt = frameToProcess.getHeader("receipt");

        boolean errorWasSent = ConnectFrameIsNotVaild(userName, acceptVersion, host, passcode, receipt);
        
        synchronized(connections){
            
            if(!errorWasSent){   // the frame is ok.
                if (!connections.doesUserExist(userName)){    // if the user does not exist- creat a new one and connect.
                    connections.createAndConnect(userName, passcode,connectionId, connectionHandler);
                    sendConnectedframe(connectionId);
                    if(receipt != null)
                        sendReceipt(connectionId, receipt);
                }
        
                else{   // the user exist.
        
                    if(connections.isUserConnected(userName)){     // the user is allready connected.
                        String errorFrame = createErrorFrame("userIsAlreadyConnected", receipt, "User already logged in", null);
                        sendError(errorFrame);
                        closeConnection();
                    }
        
                    else if(connections.UserPasswordIsCorrect(userName, passcode)){  // connect to the user.
                        connections.connect(userName, connectionId, connectionHandler);
                        sendConnectedframe(connectionId);
        
                        if(receipt != null)
                            sendReceipt(connectionId, receipt);
                    }
        
                    else{
                        String errorFrame = createErrorFrame("wrongPassword", receipt, "Wrong password", null);
                        sendError(errorFrame);
                        closeConnection();
                    }
                }
            }
        }
        
    }

    private void processDisconnect(Frame frameToProcess){
        String receipt = frameToProcess.getHeader("receipt");  
        boolean error = false;

        if(receipt == null){
            String errorFrame = createErrorFrame("NoReceiptID", receipt, "No receipt-id was found", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }
        if(!error){
            sendReceipt(connectionId, receipt);
            connections.disconnect(connectionId); //remove the user from all topics
            closeConnection(); //close the connection
        }
        

    }

    private void processSend(Frame frameToProcess){     
        String destination = frameToProcess.getHeader("destination");
        String receipt = frameToProcess.getHeader("receipt");
        String body = frameToProcess.getBody();

        boolean error = findAnErrorForSend(destination, receipt, body);

        if(!error){

            List<Integer> toSendChannel = connections.getChannelSubscribersMap().get(destination);
            for(Integer toSend : toSendChannel){
                String messageFrame = createMessageFrame(destination, body, toSend);
                connections.send(toSend, messageFrame);
            }
            messageIdCounter++;

            if(receipt != null)
                sendReceipt(connectionId, receipt);
        }

    }

    private String createMessageFrame(String channel, String body, Integer toSend){

        String subsctiptionId = connections.getUserSubscribeId(channel, toSend).toString();
        String answer = "MESSAGE" + "\n" + "subscription:" + subsctiptionId +"\n" + "message-id:" + messageIdCounter + "\n"
                         + "destination:" + channel + "\n\n" + body;
        return answer;
    }

    private void processSubscribe(Frame frameToProcess){
        String destination = frameToProcess.getHeader("destination");
        String id = frameToProcess.getHeader("id"); 
        String receipt = frameToProcess.getHeader("receipt");       

        boolean error = false;

        if(destination == null){
            String errorFrame = createErrorFrame("noDestination", receipt, "No destination header was found", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }

        if(!error && id == null){
            String errorFrame = createErrorFrame("noID", receipt, "No id header was found", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }

        if(!error){  // we have the correct data.
            if(!connections.clinetIsConnected(connectionId)){   // the client isn't connected - send error message.
                String errorFrame = createErrorFrame("userIsntConnected", receipt, "The user is not connected", null);
                sendError(errorFrame);
                closeConnection();
            }
            else{
                if(!connections.isSubscribe(destination, connectionId)){
                    connections.subscribe(destination, connectionId, Integer.valueOf(id));  // maybe check if we need to throw an error if the id is not uniqe. just for formal reasons
                }
                if(receipt != null)
                    sendReceipt(connectionId, receipt);
            }
        }

    }

    private void processUnsubscribe(Frame frameToProcess){
        String id = frameToProcess.getHeader("id"); //the id is unique for each game
        String receipt = frameToProcess.getHeader("receipt");       

        boolean error = false;

        if(id == null){
            String errorFrame = createErrorFrame("noID", receipt, "No id header was found", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }

        if(!error && !connections.clinetIsConnected(connectionId)){   // the client isn't connected - send error message.
            String errorFrame = createErrorFrame("userIsntConnected", receipt, "The user is not connected", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }

        if(!error && !connections.isSubscribe(Integer.valueOf(id), connectionId)){
            String errorFrame = createErrorFrame("userIsntSubscribed", receipt, "The id does not match any subscription id", null);
            sendError(errorFrame);
            closeConnection();
            error = true;
        }
            
        if(!error){
            connections.unsubscribe(Integer.valueOf(id), connectionId);

            if(receipt != null)
                    sendReceipt(connectionId, receipt);
        }       

    }

    private String createErrorFrame(String errorType, String receipt, String messageHeader, String moreDetailes){
        String receiptId = receipt;
        String errorFrame = "ERROR\n";
        String msgHeader = messageHeader;
        String body = moreDetailes;

        if(receiptId != null){
            errorFrame = errorFrame + "receipt-id:" + receiptId +"\n";
        }
        errorFrame = errorFrame + "message:" + msgHeader + "\n\n";

        if(body != null){
            errorFrame = errorFrame + body +"\n";
        }

        /*
         error types:
         1. client is not subscribed to the topic and tries to SEND a message to it
         2. the server cannot create the subscription when a client tries to SUBSCRIBE
         3. the client send a COMMAND and isnt logged in (beside login command)
         4. connection error- socket error' when a client tries to CONNECT (the server has fallen or wrong ip/ port) "Could not connect the server"
         5. user is already logged in- "User already logged in" (CONNECT)
         6. wrong password- "Wrong password" (CONNECT)
         7. wrong accept version
         8. wrong host
         */

        return errorFrame;
    }

    private void sendConnectedframe(int _connectionId){
        String answer = "CONNECTED" + "\n" + "version:1.2";// + "\n\n";
        connections.send(_connectionId, answer);
    }
    
    private void sendReceipt(int _connectionId, String receiptId){
        String answer = "RECEIPT" + "\n" + "receipt-id:" + receiptId;
        connections.send(_connectionId, answer);
    }

    private void sendError(String msg){
        connections.send(connectionId, msg);
    }

    /**
     * 
     * @param userName
     * @param acceptVersion
     * @param host
     * @param passcode
     * @param receipt
     * @return true iff error was sent
     */
    private boolean ConnectFrameIsNotVaild(String userName, String acceptVersion, String host, String passcode, String receipt){

        //there was no user name header
        if(userName == null){
            String errorFrame = createErrorFrame("NoLogin", receipt, "No login header was found", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        //there was no acceptVersion header
        if(acceptVersion == null){
            String errorFrame = createErrorFrame("NoAcceptVersion", receipt, "No accept-version header was found", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        //there was no host header
        if(host == null){
            String errorFrame = createErrorFrame("NoHost", receipt, "No host header was found", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        //there was no passcode header
        if(passcode == null){
            String errorFrame = createErrorFrame("NoPasscode", receipt, "No passcode header was found", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        // accept version in not correct.
        if(!acceptVersion.equals("1.2")){
            String errorFrame = createErrorFrame("wrongAcceptVersion", receipt, "Could not connect the server", "The accept version was " + acceptVersion + "but should have been 1.2");
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        // host was not correct.
        if(!host.equals("stomp.cs.bgu.ac.il")){
            String errorFrame = createErrorFrame("wrongHost", receipt, "Could not connect the server", "The port was " + host + "but should have stomp.cs.bgu.ac.il");
            sendError(errorFrame);
            closeConnection();
            return true;
        }

        // client is allready connected.
        if(connections.clinetIsConnected(connectionId)){
            String errorFrame = createErrorFrame("ClientIsAlreadyConnected", receipt, "The client is already logged in, log out before trying again", null);
            sendError(errorFrame);
            closeConnection();
            return true;

        }

        return false;
    }


    private void closeConnection(){
        shouldTerminate = true;
        // try {
        //     //connectionHandler.close();
        //     shouldTerminate = true;
        // } catch (IOException e) {
        //     e.printStackTrace();
        // }
    }

    private boolean findAnErrorForSend(String destination, String receipt, String body){
         // user is not connected
         if(!connections.isUserConnected(connectionId)){
            String errorFrame = createErrorFrame("userIsntConnected", receipt, "The user is not connected", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        // no destination header
        if(destination == null){
            String errorFrame = createErrorFrame("noDestination", receipt, "No destination header was found", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        // destination does not match any game
        if(connections.doesChannelExist(destination)){
            String errorFrame = createErrorFrame("NoSuchChannel", receipt, "The destination does not match any game", null);
            sendError(errorFrame);
            closeConnection();
            return true;
        }
         // The user did not subscribe the channel before sending the message
         if(!connections.isSubscribe(destination, connectionId)){
            String errorFrame = createErrorFrame("userIsntRegistered", receipt, "The user is not registered to the channel", "The user did not subscribe the channel before sending the message");
            sendError(errorFrame);
            closeConnection();
            return true;
        }
        // body is empty
        if(body.isEmpty()){
            String errorFrame = createErrorFrame("NoMeassage", receipt, "There is not a message to send", "The body of the message is empty");
            sendError(errorFrame);
            closeConnection();
            return true;
        }     

        return false;
    }
    
}
