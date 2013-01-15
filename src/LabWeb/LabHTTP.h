//
//  LabHTTP.h
//  Lab3
//
//  Created by Nick Porcino on 2012 10/13.
//
//

#pragma once

#include "ConcurrentQueue.h"


namespace Lab {
    
    
    void createHTTPServer(short port, concurrent_queue<std::string>* to, concurrent_queue<std::string>* from);
    void stopHTTPServer();
    
}

