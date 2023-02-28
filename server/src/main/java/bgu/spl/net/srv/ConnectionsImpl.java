package bgu.spl.net.srv;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ConnectionsImpl<T> implements Connections<T> {
    /**
     * 
     */
    //public int currentConnectionId = 1;
    private Map<Integer, ConnectionHandler<T>> connectionHandlerMap = new ConcurrentHashMap<Integer, ConnectionHandler<T>>(); // map between connectionId to ConnectionHandler.
    private Map<String, List<Integer>> channelSubscribersMap = new ConcurrentHashMap<String, List<Integer>>(); // map between channel to subscribers connectionId.
    private Map<Integer, User> UserConnectionIdMap = new ConcurrentHashMap<Integer, User>();  // map between connectionId to User.
    private Map<String, User> UserLoginMap = new ConcurrentHashMap<String, User>();  // map between login to User.
    private ConcurrentLinkedQueue<String> channelList = new ConcurrentLinkedQueue<>(); // keep list in queue of channels.

    /*
     * mapping each client to a unique ID (who the Id belongs to?)
     * Mapping every client to to his connectionHandler.
     * foreach client nedd to keep every subscription ID.
     * 
     * 
     */

    @Override
    public boolean send(int connectionId, T msg) {
        if(connectionHandlerMap.containsKey(connectionId)){
            connectionHandlerMap.get(connectionId).send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> toSendChannel = channelSubscribersMap.get(channel);
        for(Integer toSend : toSendChannel){
            send(toSend, msg);
        }
        
    }

    @Override
    public synchronized void disconnect(int connectionId) {
        for(String channel : channelList){
            if((channelSubscribersMap.get(channel)).contains(connectionId)){
                channelSubscribersMap.get(channel).remove(channelSubscribersMap.get(channel).indexOf(connectionId));
            }
        }
        connectionHandlerMap.remove(connectionId);
        UserConnectionIdMap.get(connectionId).disconnect(); // logic disconnect actions.
        UserConnectionIdMap.remove(connectionId);
       

        
    }
    
    /**
     * This method return true iff the user is connected.
     * @param connnectedId
     * @return true iff the user is connected.
     */
    public boolean isUserConnected(int connnectedId){
        return isUserConnected(UserConnectionIdMap.get(connnectedId));
    }

    /**
     * This method return true iff the user is connected.
     * @param user
     * @return true iff user is connected.
     */
    public boolean isUserConnected(User user){
        return user.isConnected();
    }

    /**
     * @param login
     * @return true iff the user is connected.
     */
    public boolean isUserConnected(String login){
        return isUserConnected(UserLoginMap.get(login));
    }

    /**
     * This function return true iff the user exsist.
     * @param login
     * @return true iff the user exsist.
     */
    public boolean doesUserExist(String login){
        return UserLoginMap.containsKey(login);  // i take an assumption that user login is uniqe.
    }

    /**
     * This medhod check if the given password is correnct.
     * @param login
     * @param password
     * @return true iff the user password is valid.
     */
    public boolean UserPasswordIsCorrect(String login, String password){
        User user = UserLoginMap.get(login);
        return user.getPassword().equals(password);
    }

    /**
     * This method checks if the client connected to some user.
     * @param connectionId
     * @return true iff the client is connected to some user.
     */
    public boolean clinetIsConnected(int connectionId){
        if(UserConnectionIdMap.containsKey(connectionId)){
            return isUserConnected(UserConnectionIdMap.get(connectionId));
        }
        return false;
    }

    /**
     * This method use to subscribe a user to a channel.
     * @param channel
     * @param connectionId
     */
    public void subscribe(String channel, int connectionId, int subscribeId){
        if(!channelList.contains(channel)){   // if there is no suck channel- creat new one.
            channelList.add(channel);
            channelSubscribersMap.put(channel, new LinkedList<>());
        }
        channelSubscribersMap.get(channel).add(connectionId);  // subscribe.
        UserConnectionIdMap.get(connectionId).subscribe(channel, subscribeId);
    }

    /**
     * unsubscribe from a channel.
     * @param subscribeId
     * @param connectionId
     */
    public void unsubscribe(int subscribeId, int connectionId){
        User user = UserConnectionIdMap.get(connectionId);  // get the user.
        String channel = user.getChannelSubscribe(subscribeId); // get the currect channel.
        if(channelSubscribersMap.get(channel).contains(connectionId))
            channelSubscribersMap.get(channel).remove(channelSubscribersMap.get(channel).indexOf(connectionId));  // delete the connectionId from the channel subscribeList.
        user.unsubscribe(channel, subscribeId);
    }

    /**
     * @param channel
     * @param connectionId
     * @return true iff the user is subscribe to the channel
     */
    public boolean isSubscribe(String channel, int connectionId){
        if(channelSubscribersMap.containsKey(channel))
            return channelSubscribersMap.get(channel).contains(connectionId);
        return false;
    }

    public boolean isSubscribe(int subscribeId, int connectionId){
        return UserConnectionIdMap.get(connectionId).isSubscribed(subscribeId);
    }

    /**
     * Create a new user and manage the data.
     * @param login
     * @param password
     * @param connectionId
     * @param connectionHandler
     */
    public void createAndConnect(String login, String password, int connectionId, ConnectionHandler<T> connectionHandler){
        User newUser = new User(login, password);
        connectionHandlerMap.put(connectionId, connectionHandler);
        UserConnectionIdMap.put(connectionId, newUser);
        UserLoginMap.put(login, newUser);
    }

    public void connect(String login, int connectionId, ConnectionHandler<T> connectionHandler){
        User user = UserLoginMap.get(login);
        connectionHandlerMap.put(connectionId, connectionHandler);
        UserConnectionIdMap.put(connectionId, user);
        
    }
    
    public boolean doesChannelExist(String channel){
        return channelList.contains(channel);
    }

    public Map<String, List<Integer>> getChannelSubscribersMap() {
        return channelSubscribersMap;
    }

    public Integer getUserSubscribeId(String channel, int subscribeId){
        return UserConnectionIdMap.get(subscribeId).getSubscribeId(channel);
    }

    
}
