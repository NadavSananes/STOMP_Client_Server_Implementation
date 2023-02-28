package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;


public class User {
    private String login;
    private String password;
    private boolean connected;
    private Map<String, Integer> channelToSubscribesId = new ConcurrentHashMap<String, Integer>(); // map the channel name and subscribeId.
    private Map<Integer, String> subscribesIdToChannels = new ConcurrentHashMap<Integer, String>(); // map the channel name and subscribeId.

    public User(String login, String password){
        this.login = login;
        this.password = password;
        connected = true;
    }

    public void subscribe(String channel, int subscribeId){
        channelToSubscribesId.put(channel, subscribeId);
        subscribesIdToChannels.put(subscribeId, channel);
    }
    
    public boolean isConnected() {
        return connected;
    }
    public String getPassword() {
        return password;
    }
    public String getLogin() {
        return login;
    }
    public void setConnected(boolean connected) {
        this.connected = connected;
    }
    public String getChannelSubscribe(int subscribeId){
        return subscribesIdToChannels.get(subscribeId);
    }
    public void unsubscribe(String channel, int subscribeId){
        channelToSubscribesId.remove(channel);
        subscribesIdToChannels.remove(subscribeId);
    }
    public void disconnect(){
        channelToSubscribesId.clear();
        subscribesIdToChannels.clear();
        connected = false;
    }
    public boolean isSubscribed(int subscribeId){
        return subscribesIdToChannels.containsKey(subscribeId);
    }
    
    public Integer getSubscribeId(String channel){
        return channelToSubscribesId.get(channel);
    }
}
