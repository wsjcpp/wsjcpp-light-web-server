#ifndef WSJCPP_LIGHT_WEB_SERVER_H
#define WSJCPP_LIGHT_WEB_SERVER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <map>
#include <deque>
#include <mutex>
#include <vector>

#include "wsjcpp_light_web_http_response.h"
#include "wsjcpp_light_web_http_request.h"

// ---------------------------------------------------------------------

class WSJCppLightWebHttpDequeRequests {
    public:
        WSJCppLightWebHttpRequest *popRequest();
        void pushRequest(WSJCppLightWebHttpRequest *pRequest);
        void cleanup();

    private:
        std::string TAG;

        std::mutex m_mtxDequeRequests;
        std::mutex m_mtxWaiterRequests;
        std::deque<WSJCppLightWebHttpRequest *> m_dequeRequests;
};

// ---------------------------------------------------------------------

class WSJCppLightWebHttpHandlerBase {
    public:
        WSJCppLightWebHttpHandlerBase(const std::string &sName);
        const std::string &name();
        virtual bool canHandle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest) = 0;
        virtual bool handle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest) = 0;

    private:
        std::string m_sName;
};

// ---------------------------------------------------------------------

class WSJCppLightWebHttpHandlers {
    public:
        WSJCppLightWebHttpHandlers();
        void add(WSJCppLightWebHttpHandlerBase *pHandler);
        void remove(const std::string &sName);
        bool find(const std::string &sName, WSJCppLightWebHttpHandlerBase *pHandler);
        bool handle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest);
    private:
        std::string TAG;
        std::vector<WSJCppLightWebHttpHandlerBase *> m_pHandlers;
};

// ---------------------------------------------------------------------

class WSJCppLightWebHttpThreadWorker {
    public:

        WSJCppLightWebHttpThreadWorker(
            const std::string &sName, 
            WSJCppLightWebHttpDequeRequests *pDeque, 
            WSJCppLightWebHttpHandlers *pHandlers
        );

        void start();
        void stop();
        void run();        
    private:
        std::string TAG;
        std::string m_sName;
        WSJCppLightWebHttpDequeRequests *m_pDeque;
        WSJCppLightWebHttpHandlers *m_pHandlers;
        bool m_bStop;
        pthread_t m_serverThread;
};

// ---------------------------------------------------------------------

class LightHttpServer {
    public:

        LightHttpServer();
        void setPort(int nPort);
        void setMaxWorkers(int nMaxWorkers);
        void startSync();
        void start();
        void stop();

        WSJCppLightWebHttpHandlers *handlers();

    private:
        std::string TAG;
        WSJCppLightWebHttpDequeRequests *m_pDeque;
        WSJCppLightWebHttpHandlers *m_pHandlers;
        bool m_bStop;

        int m_nMaxWorkers;
        int m_nPort;
        std::vector<WSJCppLightWebHttpThreadWorker *> m_vWorkers;

        int m_nSockFd;
        struct sockaddr_in m_serverAddress;
        pthread_t m_serverThread;
};
#endif // WSJCPP_LIGHT_WEB_SERVER_H


