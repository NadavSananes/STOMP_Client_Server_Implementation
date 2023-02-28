package bgu.spl.net.impl.stomp;
//mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="7777 tpc"
public class Frame {

    private String command;
    private String[][] headers;
    private String body;

    public Frame (String msg){
        String[] splittedMsg = msg.split("\n\n", 2); //return the command and the headers (if they exist) in the first cell and the body in the second
        body = splittedMsg[1]; //if there is no body the string is empty
        String[] secSplit = splittedMsg[0].split("\n", 2); //return the command in the first cell and the headrers in the second
        command = secSplit[0];
        if(secSplit.length == 2){ //there are headers
            String[] splitHeaders = secSplit[1].split("\n"); //return a array of the stings belong to headrers
            headers = new String[splitHeaders.length][];

            for(int i=0; i < splitHeaders.length; i++){
                headers[i] = splitHeaders[i].split(":");
            }
        }   
            
    }

    public String getCommand() {
        return command;
    }

    public String[][] getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }


    /**
     * look for the header we want.
     * @param str
     * @return the header str or null if if does not exist.
     */
    public String getHeader(String str){
        if(headers == null || headers.length == 0){  // there is no suck header.
            return null;
        }
        else{
            for(int i = 0; i < headers.length; i = i + 1){
                if(headers[i][0].equals(str)){
                    return headers[i][1];   // found the header.
                }
            }
        }
        return null;   //dont found the header. 
    }

    
}
