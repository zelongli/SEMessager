package client;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
	
public class Client {
	FileInputStream fileInputStream;
	DataInputStream netInputStream;
	DataOutputStream netOutputStream;
	Socket socket;
	int fileLength;
	byte[] buffer = new byte[1024];
	byte[] readLen = new byte[10];
	byte[] readResult = new byte[128];
	int len;
	int result_cout = 0;
	
	public int uglyhash(byte[] message){
			int hash = 0;
			
			for(int i = 0; i< message.length; ++i) {
				hash += message[i];
			}
			
			return hash;
	}
	
	public byte[] uglyEncrpt(int hash, byte[] key) {
		byte[] hashbyte = ByteBuffer.allocate(4).putInt(hash).array();
		for(int i = 0; i<4; ++i) {
			hashbyte[i] ^= key[i];
		}
		return hashbyte;
	}
	
	void query(String message, String ip, int port) {
		byte[] messageData;
		byte[] key = {(byte)192,(byte)168,(byte)1,(byte)1};
		try{
			socket = new Socket(ip, port);
			netInputStream = new DataInputStream(socket.getInputStream());
			netOutputStream = new DataOutputStream(socket.getOutputStream());
			netOutputStream.write(message.getBytes("UTF-8"));
			
			int hashValue = uglyhash(message.getBytes());
			System.out.println("get hash "+ hashValue);
			netOutputStream.write(uglyEncrpt(hashValue,key));
			
//			int result = netInputStream.read();
//			System.out.println(result);
			
		}catch(Exception e){
			e.printStackTrace();		
		}
		
	}
}
