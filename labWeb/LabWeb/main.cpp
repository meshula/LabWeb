//
//  main.cpp
//  LabWeb
//
//  Created by Nick Porcino on 2011 04/10.
//

#include "LabWebSocket.h"

#include <iostream>

int main (int argc, const char * argv[])
{
	// try http://websocket.org/echo.html, and connect to ws://localhost:1234
	// you can find the @TODO in the websocket code and put a cout there.
    webSocketInterface(1234);
    while (1) sleep(10);    
    return 0;
}

