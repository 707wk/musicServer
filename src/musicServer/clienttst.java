package musicServer;

import java.io.BufferedReader ;
import java.io.IOException ;
import java.io.InputStream ;
import java.io.InputStreamReader ;
import java.io.OutputStream ;
import java.io.PrintWriter ;
import java.net.InetAddress;
import java.net.Socket ;
import java.net.UnknownHostException ;

public class clienttst 
{
    
    public static void main(String[]args)
    {
        try 
        {
        	InetAddress.getByName("192.168.152.6").isReachable(3000);
            Socket socket=new Socket("192.168.152.6",1707);
            OutputStream os=socket.getOutputStream();
            PrintWriter pw=new PrintWriter(os);
            pw.write("connect");
            pw.flush();
            socket.shutdownOutput();
            InputStream is=socket.getInputStream();
            BufferedReader br=new BufferedReader(new InputStreamReader(is));
            String info=null ;
            info=br.readLine();
            System.out.println("rec£º"+info);
            br.close();
            is.close();
            pw.close();
            os.close();
            socket.close();
            System.out.println("end");
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }
    
}