package musicServer;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class testget {
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
            pw.write("GET '009.mp3'\n");
            pw.flush();
            //netout = new DataOutputStream(client.getOutputStream());
            //netout.writeChars("GET 003.mp3");
            String filesname=br.readLine();
            if(filesname.equals("NULL")){
            	System.out.println("not found files "+filesname);
            	return ;
            }else{
            	System.out.println("files: ["+filesname+"]");
            }
            
            Double length=Double.valueOf(br.readLine()); 
            System.out.println("length: ["+length+"]");
            filesoutput=new FileOutputStream(new File(filesname));  
            
            int nums=0;
            double recsum=0;
            byte[]sendBytes=new byte[1024/16];
            for(;;){  
                int read=0 ;  
                read=netin.read(sendBytes);
                //System.out.printf("[ %8d-%5d]",nums,read);
                if(read==-1)  
                    break ;  
                recsum+=read;
                filesoutput.write(sendBytes,0,read);
                //pw.write("ok");
                nums++;
            }  
            filesoutput.close();  
            netin.close();  
            client.close(); 
            System.out.printf("\n[ %8d-%10.0f]\n",nums,recsum);
        }  
        catch(Exception e){  
            e.printStackTrace();  
        	System.out.println("数据接收异常");  
        }  
    }  
}
