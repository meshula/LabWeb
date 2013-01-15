//
//  LabWebSocket.h
//  LabWeb
//
//  Created by Nick Porcino on 2011 04/10.
//

#pragma once

#include "ConcurrentQueue.h"
#include <string>

void webSocketInterface(short port, int verbosity, concurrent_queue<std::string>* to, concurrent_queue<std::string>* from);
