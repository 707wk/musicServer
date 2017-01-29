package musicServer;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class testput {
	public static void main(String[]args){  
        Socket client=null ;  
        FileInputStream filesoutput=null ;  
        DataInputStream netin=null ; 
        DataOutputStream netout = null ;
          
        try {  
            client=new Socket("192.168.152.6",8580);  
            //netin=new DataInputStream(client.getInputStream()); 
            PrintWriter pw=new PrintWriter(client.getOutputStream());
            BufferedReader br=new BufferedReader(new InputStreamReader(client.getInputStream()));
            pw.write("PUT '0013.mp3' '未知8' '无8' '4:56' '1234568'");
            pw.flush();
            netout = new DataOutputStream(client.getOutputStream());
            //netout.writeChars("GET 003.mp3");
            String filesname=br.readLine();
            if(filesname.equals("OK")){
            	System.out.println("ok");
            }else{
            	System.out.println("repetition");
            	return ;
            }
            
            filesoutput=new FileInputStream(new File("008.mp3")); 
            
            int nums=0;
            byte[]sendBytes=new byte[1024/16];
            int length = 0 ;
            for(;(length = filesoutput.read(sendBytes, 0, sendBytes.length))>0;){
                netout.write(sendBytes, 0, length);
                netout.flush();
                nums++;
            }  
            filesoutput.close();  
            netout.close();  
            client.close(); 
            System.out.printf("\n[ %8d]\n",nums);
        }  
        catch(Exception e){  
            e.printStackTrace();  
        	System.out.println("数据接收异常");  
        }  
    }
}
