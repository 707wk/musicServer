package musicServer;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class testdel {
	public static void main(String[]args){  
        Socket client=null ;  
        FileOutputStream filesoutput=null ;  
        DataInputStream netin=null ; 
        DataOutputStream netout = null ;
          
        try {  
            client=new Socket("192.168.152.6",8580);  
            netin=new DataInputStream(client.getInputStream()); 
            PrintWriter pw=new PrintWriter(client.getOutputStream());
            BufferedReader br=new BufferedReader(new InputStreamReader(client.getInputStream()));
            pw.write("DEL '001中.mp3'");
            pw.flush();
            System.out.println("rec ["+br.readLine()+"]"); 
            netin.close();  
            client.close(); 
        }  
        catch(Exception e){  
            e.printStackTrace();  
        	System.out.println("数据接收异常");  
        }  
    }
}
