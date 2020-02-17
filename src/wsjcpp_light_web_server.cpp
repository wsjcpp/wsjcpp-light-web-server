#include "wsjcpp_light_web_server.h"

#include <sstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <fstream>
#include <regex>        // regex, sregex_token_iterator
#include <stdio.h>
#include <math.h>
#include <thread>
#include <algorithm>
#include <wsjcpp_core.h>

// ----------------------------------------------------------------------
// WSJCppLightWebHttpDequeRequests

WSJCppLightWebHttpRequest *WSJCppLightWebHttpDequeRequests::popRequest() {
    if (m_dequeRequests.size() == 0) {
        m_mtxWaiterRequests.lock();
    }
    std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
    WSJCppLightWebHttpRequest *pRequest = nullptr;
    int nSize = m_dequeRequests.size();
    if (nSize > 0) {
        pRequest = m_dequeRequests.back();
        m_dequeRequests.pop_back();
    }
    return pRequest;
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpDequeRequests::pushRequest(WSJCppLightWebHttpRequest *pRequest) {
    {
        std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
        if (m_dequeRequests.size() > 20) {
            WSJCppLog::warn(TAG, " deque more than " + std::to_string(m_dequeRequests.size()));
        }
        m_dequeRequests.push_front(pRequest);
    }
    
    if (m_dequeRequests.size() == 1) {
        m_mtxWaiterRequests.unlock();   
    }
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpDequeRequests::cleanup() {
    std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
    while (m_dequeRequests.size() > 0) {
        delete m_dequeRequests.back();
        m_dequeRequests.pop_back();
    }
}

// ---------------------------------------------------------------------
// WSJCppLightWebHttpHandlerBase

WSJCppLightWebHttpHandlerBase::WSJCppLightWebHttpHandlerBase(const std::string &sName) {
    m_sName = sName;
}

// ---------------------------------------------------------------------

const std::string &WSJCppLightWebHttpHandlerBase::name() {
    return m_sName;
}

// ----------------------------------------------------------------------
// WSJCppLightWebHttpHandlers

WSJCppLightWebHttpHandlers::WSJCppLightWebHttpHandlers() {
    TAG = "WSJCppLightWebHttpHandlers";
}

// ---------------------------------------------------------------------

void WSJCppLightWebHttpHandlers::add(WSJCppLightWebHttpHandlerBase *pHandler) {
    m_pHandlers.push_back(pHandler);
}

// ---------------------------------------------------------------------

void WSJCppLightWebHttpHandlers::remove(const std::string &sName) {
    std::vector<WSJCppLightWebHttpHandlerBase *>::iterator it = m_pHandlers.begin();
    for (it = m_pHandlers.end(); it != m_pHandlers.begin(); it--) {
        WSJCppLightWebHttpHandlerBase *pHandler = *it;
        if (pHandler->name() == sName) {
            m_pHandlers.erase(it);
        }
    }
}

// ---------------------------------------------------------------------

bool WSJCppLightWebHttpHandlers::find(const std::string &sName, WSJCppLightWebHttpHandlerBase *pHandler) {
    std::vector<WSJCppLightWebHttpHandlerBase *>::iterator it = m_pHandlers.begin();
    for (it = m_pHandlers.begin(); it != m_pHandlers.end(); ++it) {
        WSJCppLightWebHttpHandlerBase *p = *it;
        if (p->name() == sName) {
            pHandler = p;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------

bool WSJCppLightWebHttpHandlers::handle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest) {
    std::string _tag = TAG + "-" + sWorkerId;

    for (int i = 0; i < m_pHandlers.size(); i++) {
        if (m_pHandlers[i]->canHandle(sWorkerId, pRequest)) {
            // TODO check multi handlers -> throw error
            if (m_pHandlers[i]->handle(sWorkerId, pRequest)) {
                return true;
            } else {
                WSJCppLog::warn("LightHttpHandlers", m_pHandlers[i]->name() + " - could not handle request '" + pRequest->requestPath() + "'");
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------
// WSJCppLightWebHttpThreadWorker

void* processRequest(void *arg) {
    WSJCppLightWebHttpThreadWorker *pWorker = (WSJCppLightWebHttpThreadWorker *)arg;
    pthread_detach(pthread_self());
    pWorker->run();
    return 0;
}

// ----------------------------------------------------------------------

WSJCppLightWebHttpThreadWorker::WSJCppLightWebHttpThreadWorker(
    const std::string &sName, 
    WSJCppLightWebHttpDequeRequests *pDeque, 
    WSJCppLightWebHttpHandlers *pHandlers
) {
    TAG = "WSJCppLightWebHttpThreadWorker-" + sName;
    m_pDeque = pDeque;
    m_bStop = false;
    m_sName = sName;
    m_pHandlers = pHandlers;
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpThreadWorker::start() {
    m_bStop = false;
    pthread_create(&m_serverThread, NULL, &processRequest, (void *)this);
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpThreadWorker::stop() {
    m_bStop = true;
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpThreadWorker::run() {
    const int nMaxPackageSize = 4096;
    while (1) {
        if (m_bStop) return;
        WSJCppLightWebHttpRequest *pInfo = m_pDeque->popRequest();
        bool bExists = pInfo != nullptr;
        // TODO refactor
        if (bExists) {
            int nSockFd = pInfo->sockFd();

            // set timeout options
            struct timeval timeout;
            timeout.tv_sec = 1; // 1 seconds timeout
            timeout.tv_usec = 0;
            setsockopt(nSockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            int res = getpeername(nSockFd, (struct sockaddr *)&addr, &addr_size);
            char *clientip = new char[20];
            memset(clientip, 0, 20);
            strcpy(clientip, inet_ntoa(addr.sin_addr));
            WSJCppLog::info(TAG, "IP-address: " + std::string(clientip));

            WSJCppLightWebHttpResponse *pResponse = new WSJCppLightWebHttpResponse(nSockFd);
            int n;
            // int newsockfd = (long)arg;
            char msg[nMaxPackageSize];

            std::string sRequest;
            
            // std::cout << nSockFd  << ": address = " << info->address() << "\n";
            // read data from socket
            bool bErrorRead = false;
            while (1) { // problem can be here
                // std::cout << nSockFd  << ": wait recv...\n";
                memset(msg, 0, nMaxPackageSize);

                n = recv(nSockFd, msg, nMaxPackageSize, 0);
                // std::cout << "N: " << n << std::endl;
                if (n == -1) {
                    bErrorRead = true;
                    std::cout << nSockFd  << ": error read... \n";
                    break;
                }
                if (n == 0) {
                    //close(nSockFd);
                    break;
                }
                WSJCppLog::info(TAG, "Readed " + std::to_string(n) + " bytes...");
                msg[n] = 0;
                //send(newsockfd,msg,n,0);
                sRequest = std::string(msg);

                std::string sRecv(msg);
                pInfo->appendRecieveRequest(sRecv);

                if (pInfo->isEnoughAppendReceived()) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            WSJCppLog::info(TAG, "\nRequest: \n>>>\n" + sRequest + "\n<<<");

            if (bErrorRead) {
                pResponse->sendRequestTimeOut();
            } else if (pInfo->requestType() == "OPTIONS") {
                pResponse->ok().sendOptions("OPTIONS, GET, POST");
            } else if (pInfo->requestType() != "GET" && pInfo->requestType() != "POST") {
                pResponse->notImplemented().sendEmpty();
            } else {
                if (!m_pHandlers->handle(m_sName, pInfo)) {
                    pResponse->notFound().sendEmpty();
                } else {
                    // TODO resp internal error
                    // this->response(WSJCppLightWebHttpResponse::RESP_INTERNAL_SERVER_ERROR);     
                }
            }
            delete pInfo;
            delete pResponse;
        }

        /*if (!bExists) {
            if (m_bStop) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (m_bStop) return;
        }*/
    }
}


// ----------------------------------------------------------------------
// LightHttpServer

LightHttpServer::LightHttpServer() {
    TAG = "LightHttpServer";
    m_nMaxWorkers = 4;
    m_pDeque = new WSJCppLightWebHttpDequeRequests();
    m_pHandlers = new WSJCppLightWebHttpHandlers();
    m_bStop = false;
    m_nPort = 8080;
}

// ----------------------------------------------------------------------

void LightHttpServer::setPort(int nPort) {
    if (nPort > 10 && nPort < 65536) {
        m_nPort = nPort;
    } else {
        WSJCppLog::throw_err(TAG, "Port must be 10...65535");
    }
    m_nPort = nPort;
}

// ----------------------------------------------------------------------

void LightHttpServer::setMaxWorkers(int nMaxWorkers) {
    if (nMaxWorkers > 0 && nMaxWorkers <= 100) {
        m_nMaxWorkers = nMaxWorkers;
    } else {
        WSJCppLog::warn(TAG, "Max workers must be 1...100");
    }
}

// ----------------------------------------------------------------------

void LightHttpServer::startSync() {
    
    m_nSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nSockFd <= 0) {
        WSJCppLog::err(TAG, "Failed to establish socket connection");
        return;
    }
    int enable = 1;
    if (setsockopt(m_nSockFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        WSJCppLog::err(TAG, "setsockopt(SO_REUSEADDR) failed");
        return;
    }

     memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverAddress.sin_port = htons(m_nPort);
    if (bind(m_nSockFd, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress)) == -1) {
        WSJCppLog::err(TAG, "Error binding to port " + std::to_string(m_nPort));
        return;
    }
    listen(m_nSockFd, 5);
    WSJCppLog::info("LightHttpServer", "Light Http Server started on " + std::to_string(m_nPort) + " port.");

    for (int i = 0; i < m_nMaxWorkers; i++) {
        m_vWorkers.push_back(new WSJCppLightWebHttpThreadWorker("worker" + std::to_string(i), m_pDeque, m_pHandlers));
    }

    for (int i = 0; i < m_vWorkers.size(); i++) {
        m_vWorkers[i]->start();
    }

    std::string str;
    m_bStop = false;
    while (!m_bStop) { // or problem can be here
        struct sockaddr_in clientAddress;
        socklen_t sosize  = sizeof(clientAddress);
        int nSockFd = accept(m_nSockFd,(struct sockaddr*)&clientAddress,&sosize);
        std::string sAddress = inet_ntoa(clientAddress.sin_addr);
        WSJCppLightWebHttpRequest *pInfo = new WSJCppLightWebHttpRequest(nSockFd, sAddress);
        // info will be removed inside a thread
        m_pDeque->pushRequest(pInfo);

        // pthread_create(&m_serverThread, NULL, &newRequest, (void *)pInfo);
        // std::cout << "wait \n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    m_pDeque->cleanup();
    for (int i = 0; i < m_vWorkers.size(); i++) {
        m_vWorkers[i]->stop();
    }
    close(m_nSockFd);
}

// ----------------------------------------------------------------------

void* processWebServerStart(void *arg) {
    LightHttpServer *pWebServer = (LightHttpServer *)arg;
    pthread_detach(pthread_self());
    pWebServer->startSync();
    return 0;
}

// ----------------------------------------------------------------------

void LightHttpServer::start() {
    m_bStop = false;
    pthread_create(&m_serverThread, NULL, &processWebServerStart, (void *)this);
}
// ----------------------------------------------------------------------

WSJCppLightWebHttpHandlers *LightHttpServer::handlers() {
    return m_pHandlers;
}

// ----------------------------------------------------------------------

void LightHttpServer::stop() {
    m_bStop = true;
} 