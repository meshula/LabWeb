//
//  LabWebSocket.cpp
//  LabWeb
//
//  Created by Nick Porcino on 2011 04/10.
//

#include "LabWebSocket.h"
#include "LabMD5.h"

#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;
const int max_length = 1024;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

namespace {
	
	const char* responseStr = 
    "HTTP/1.1 101 WebSocket Protocol Handshake\r\n"
    "Upgrade: WebSocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Origin: %s\r\n"
    "Sec-WebSocket-Location: ws%s://%s%s\r\n"
    "Sec-WebSocket-Protocol: sample\r\n\r\n\%s";
	
	
    bool handshakeWebSocket(const char* buffer, char* responseBuff)
    {
        bool isWebSocket = false;
        if (!strncmp(buffer, "GET ", 4)) {
            //--------------------------------
            // PATH
            //
            const char* curr = buffer + 4;
            const char* end = strstr(curr, " HTTP/1.1");
            if (!end)
                return false;
            
            std::string path(curr, end - curr);
            
            //--------------------------------
            // HOST
            //
            curr = strstr(buffer, "\r\nHost: ");
            if (!curr)
                return false;
            
            curr += 8;
            end = strstr(curr, "\r\n");
            if (!end)
                return false;
            
            std::string host(curr, end - curr);
            
            //--------------------------------
            // ORIGIN
            //
            curr = strstr(buffer, "\r\nOrigin: ");
            if (!curr)
                return false;
            
            curr += 10;
            end = strstr(curr, "\r\n");
            if (!end)
                return false;
            
            std::string origin(curr, end - curr);
            
            //--------------------------------
            // KEY1
            //
            curr = strstr(buffer, "\r\nSec-WebSocket-Key1: ");
            if (!curr)
                return false;
            
            curr += 22;
            end = strstr(curr, "\r\n");
            if (!end)
                return false;
            
            std::string key1(curr, end - curr);
            
            //--------------------------------
            // KEY2
            //
            curr = strstr(buffer, "\r\nSec-WebSocket-Key2: ");
            if (!curr)
                return false;
            
            curr += 22;
            end = strstr(curr, "\r\n");
            if (!end)
                return false;
            
            std::string key2(curr, end - curr);
            
            //--------------------------------
            // KEY3
            //
            curr = strstr(buffer, "\r\n\r\n");
            if (!curr)
                return false;
            
            curr += 4;
            if (strlen(curr) != 8)
                return false;
            
            std::string key3(curr, 8);
            
            int key1spaces = 0;
            unsigned int key1Num = 0;
            for (int i = 0; i < key1.length(); ++i) {
                key1spaces += key1[i] == ' ' ? 1 : 0;
                if (key1[i] >= '0' && key1[i] <= '9') {
                    key1Num = key1Num * 10 + (key1[i] - '0');
                }
            }
            key1Num = key1Num / key1spaces;
            
            int key2spaces = 0;
            unsigned int key2Num = 0;
            for (int i = 0; i < key2.length(); ++i) {
                key2spaces += key2[i] == ' ' ? 1 : 0;
                if (key2[i] >= '0' && key2[i] <= '9') {
                    key2Num = key2Num * 10 + (key2[i] - '0');
                }
            }
            key2Num = key2Num / key2spaces;
            
            unsigned char  buff[16];
            unsigned char* num = reinterpret_cast<unsigned char*>(&key1Num);
            buff[0] = num[3];
            buff[1] = num[2];
            buff[2] = num[1];
            buff[3] = num[0];
            num = reinterpret_cast<unsigned char*>(&key2Num);
            buff[4] = num[3];
            buff[5] = num[2];
            buff[6] = num[1];
            buff[7] = num[0];
            for (int i = 0; i < 8; ++i)
                buff[i+8] = key3[i];
            
            unsigned char digest[16];
            MD5_CTX md5;
            MD5Init(&md5);
            MD5Update(&md5, buff, 16);
            MD5Final(digest, &md5);
            
            sprintf(responseBuff, responseStr,
                    origin.c_str(), "",
                    host.c_str(),
                    path.c_str(),
                    digest);
            
            isWebSocket = true;
        }
        return isWebSocket;
    }
    
    void session(socket_ptr sock)
    {
        try {
            for (;;) {
                char data[max_length];
                
                boost::system::error_code error;
                size_t length = sock->read_some(boost::asio::buffer(data), error);
                if (error == boost::asio::error::eof)
                    break; // Connection closed cleanly by peer.
                else if (error)
                    throw boost::system::system_error(error); // Some other error.
                
                char responseBuff[256];
                if (handshakeWebSocket(data, responseBuff)) {
                    boost::asio::write(*sock, boost::asio::buffer(responseBuff, strlen(responseBuff)));
                }
                else {
                    // strip off the surrounding 0 & 0xff
                    std::string temp(data + 1, length - 2);
                    /// userFunction(temp); /// @TODO invoke a user function with the string
                    //boost::asio::write(*sock, boost::asio::buffer(data, length));
                }
            }
        }
        catch (std::exception& e) {
            std::cerr << "Exception in thread: " << e.what() << "\n";
        }
    }
    
    void server(boost::asio::io_service& io_service, short port)
    {
        tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
        for (;;) {
            socket_ptr sock(new tcp::socket(io_service));
            a.accept(*sock);
            boost::thread t(boost::bind(session, sock));
        }
    }
	
} // anon

void webSocketInterface(short port)
{
	try {
		boost::asio::io_service io_service;
		server(io_service, port);
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

