//
//  LabWebSocket.cpp
//  LabWeb
//
//  Created by Nick Porcino on 2011 04/10.
//

#include "LabWebSocket.h"

#include "websocketpp.hpp"

#include <thread>


namespace {
    // websocket server completely contained here
    
    using websocketpp::server;
    server::handler::ptr handler;
    
    int wsVerbosity = 0;
    bool wsRunning = true;
	concurrent_queue<std::string>* queueFromServer = 0;
	concurrent_queue<std::string>* queueToServer = 0;
    
    class echo_server_handler : public server::handler {
    public:
        void on_message(connection_ptr con, message_ptr msg) {
            
            if (queueFromServer && queueToServer) {
                queueFromServer->push(msg->get_payload());
                
                std::string result;
                queueToServer->wait_and_pop(result);
                
                con->send(result, msg->get_opcode());
            }
            else
                con->send(msg->get_payload(), msg->get_opcode());
        }
    };
    
    void webSocketInterfaceRun2(short port)
    {
        while (wsRunning) {
            try {
                server::handler::ptr h(new echo_server_handler());
                server echo_endpoint(h);
                
                echo_endpoint.alog().unset_level(websocketpp::log::alevel::ALL);
                echo_endpoint.elog().unset_level(websocketpp::log::elevel::ALL);
                
                echo_endpoint.alog().set_level(websocketpp::log::alevel::CONNECT);
                
                echo_endpoint.elog().set_level(websocketpp::log::elevel::RERROR);
                echo_endpoint.elog().set_level(websocketpp::log::elevel::FATAL);
                
                std::cout << "Starting WebSocket echo server on port " << port << std::endl;
                echo_endpoint.listen(port);
                
                handler = h;
            }
            catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << "\n";
            }
            
            int milliseconds = 100;
#if defined(_MSC_VER)
            ::Sleep( static_cast<int>( milliseconds ) );
#else
            useconds_t microsecs = milliseconds * 1000;
            ::usleep( microsecs );
#endif
        }
    }
} // anon

void webSocketInterface(short port, int _verbosity,
                        concurrent_queue<std::string>* _queueToServer,
                        concurrent_queue<std::string>* _queueFromServer)
{
    wsRunning = true;
    wsVerbosity = _verbosity;
    queueToServer = _queueToServer;
    queueFromServer = _queueFromServer;
    /*std::thread* t = */ new std::thread(boost::bind(webSocketInterfaceRun2, port));
}
