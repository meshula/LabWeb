//
//  LabHTTP.cpp
//  Lab3
//
//  Created by Nick Porcino on 2012 10/13.
//
//

#include "LabHTTP.h"

#include "mongoose.h"

#include <string>
#include <thread>
#include <string.h>
#include <boost/bind.hpp>

namespace {
    
    //const char* data = "data";
    concurrent_queue<std::string>* queueFromServer = 0;
    concurrent_queue<std::string>* queueToServer = 0;
    struct mg_context* ctx = 0;
    
    static void *mongoose_callback(enum mg_event ev, struct mg_connection *conn)
    {
        // Mongoose tries to open file.
        // If callback returns non-NULL, Mongoose will not try to open it, but
        // will use the returned value as a pointer to the file data. This allows
        // for example to serve files from memory.
        // ev_data contains file path, including document root path.
        // Upon return, ev_data should return file size,  which should be a long int.
        //
        if (ev == MG_OPEN_FILE) {
            mg_request_info* request_info = mg_get_request_info(conn);
            //const char* file_name = (const char *) request_info->ev_data;
            if (strcmp(request_info->uri+1, "foo.txt") == 0) {      // +1 because of leading /
                
                std::string request(request_info->uri+1);
                queueFromServer->push(request);
                static std::string result;
                queueToServer->wait_and_pop(result);
                
                if (result.empty())
                    return 0;
                
                request_info->ev_data = (void *) (long) result.size();
                return (void*) result.c_str();  // will mongoose consume result immediately? Is there a re-entrancy problem here?
            }
            
            // Returning NULL marks request as not handled, signalling mongoose to
            // proceed with handling it.
            return 0;
        }
        else if (ev == MG_EVENT_LOG) {
            printf("%s\n", (const char *) mg_get_request_info(conn)->ev_data);
        }
        
        // Returning NULL marks request as not handled, signalling mongoose to
        // proceed with handling it.
        return NULL;
    }

    void runHTTPServer(short port)
    {
    }
    
}

namespace Lab {

    
    void createHTTPServer(short port, concurrent_queue<std::string>* to, concurrent_queue<std::string>* from)
    {
        queueToServer = to;
        queueFromServer = from;
        
        char portStr[8];
        sprintf(portStr, "%d", (int) port);
        const char *options[] = {"listening_ports", portStr,
            "document_root", "/users/dp/Sites",
            NULL};
        
        ctx = mg_start(&mongoose_callback, NULL, options);
    }
    
    void stopHTTPServer()
    {
        if (ctx)
            mg_stop(ctx);
    }


}
