#include "wsjcpp_light_web_server.h"
#include <unistd.h>
#include <string.h>
#include <wsjcpp_core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// ---------------------------------------------------------------------
// WsjcppLightWebHttpHandlerBase

WsjcppLightWebHttpHandlerBase::WsjcppLightWebHttpHandlerBase(const std::string &sName) {
    m_sName = sName;
}

// ---------------------------------------------------------------------

const std::string &WsjcppLightWebHttpHandlerBase::name() {
    return m_sName;
}

// ----------------------------------------------------------------------
// WsjcppLightWebHttpThreadWorker

void* wsjcppLightWebServerProcessRequest(void *arg) {
    WsjcppLightWebHttpThreadWorker *pWorker = (WsjcppLightWebHttpThreadWorker *)arg;
    pthread_detach(pthread_self());
    pWorker->run();
    return 0;
}

// ----------------------------------------------------------------------

WsjcppLightWebHttpThreadWorker::WsjcppLightWebHttpThreadWorker(
    const std::string &sName, 
    WsjcppLightWebDequeHttpRequests *pDeque, 
    std::vector<WsjcppLightWebHttpHandlerBase *> *pVHandlers,
    bool bLoggerEnabled
) {
    TAG = "WsjcppLightWebHttpThreadWorker-" + sName;
    m_pDeque = pDeque;
    m_bStop = false;
    m_bStopped = true;
    m_sName = sName;
    m_pVHandlers = pVHandlers;
    m_bLoggerEnabled = bLoggerEnabled;
}

// ----------------------------------------------------------------------

void WsjcppLightWebHttpThreadWorker::start() {
    m_bStop = false;
    m_bStopped = false;
    if (m_bLoggerEnabled) {
        WsjcppLog::info(TAG, "Start");
    }
    pthread_create(&m_serverThread, NULL, &wsjcppLightWebServerProcessRequest, (void *)this);
}

// ----------------------------------------------------------------------

void WsjcppLightWebHttpThreadWorker::stop() {
    m_bStop = true;
}

// ----------------------------------------------------------------------

void WsjcppLightWebHttpThreadWorker::run() {
    if (m_bLoggerEnabled) {
        WsjcppLog::info(TAG, "Running");
    }
    const int nMaxPackageSize = 4096;
    while (1) {
        if (m_bStop) {
            m_bStopped = true;
            return;
        }
        WsjcppLightWebHttpRequest *pRequest = m_pDeque->popRequest();
        
        if (pRequest != nullptr) {
            int nSockFd = pRequest->getSockFd();
            if (m_bLoggerEnabled) {
                WsjcppLog::info(TAG, "IP-address: " + pRequest->getAddress());
            }

            // set timeout options
            struct timeval timeout;
            timeout.tv_sec = 1; // 1 seconds timeout
            timeout.tv_usec = 0;
            int nResult = setsockopt(nSockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            // TODO check nResult
            // setsockopt() with SO_NOSIGPIPE

            WsjcppLightWebHttpResponse *pResponse = new WsjcppLightWebHttpResponse(nSockFd, m_bLoggerEnabled);
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
                    WsjcppLog::err(TAG,  std::to_string(nSockFd) + ": error read... after: " + pRequest->getRequestData());
                    break;
                } else if (n == 0) {
                    // close(nSockFd);
                    break;
                }
                if (m_bLoggerEnabled) {
                    WsjcppLog::info(TAG, "Readed " + std::to_string(n) + " bytes...");
                }
                
                msg[n] = 0;
                sRequest = std::string(msg);

                std::string sRecv(msg);
                pRequest->appendRecieveRequest(sRecv);

                if (pRequest->isEnoughAppendReceived()) {
                    break;
                }
                // TODO redesign or switch to another socket
                // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            // TODO read and replace X-Forwarded-IP
            // TODO read and replace X-Forwarded-Host
            if (m_bLoggerEnabled) {
                WsjcppLog::info(TAG, "\nRequest: \n>>>\n" + sRequest + "\n<<<");
            }
            if (bErrorRead) {
                pResponse->requestTimeout().noCache().sendText(
                    "<html><body><h1>408 Request Time-out</h1>"
                    "Your browser didn't send a complete request in time."
                    "</body></html>"
                );
            } else if (pRequest->getRequestType() == "OPTIONS") {
                pResponse->ok().sendOptions("OPTIONS, GET, POST");
            } else if (pRequest->getRequestType() != "GET" && pRequest->getRequestType() != "POST") {
                pResponse->notImplemented().sendEmpty();
            } else {
                this->handle(pRequest, pResponse);
            }
            if (pRequest->getHeaderConnection() == "keep-alive") {
                // m_pDeque->addKeepAliveSocket(pRequest->getSockFd());
                close(pRequest->getSockFd());
            } else {
                close(pRequest->getSockFd());
            }
            delete pRequest;
            delete pResponse;
        }
        if (m_bStop) {
            m_bStopped = true;
            return;
        }
    }
    m_bStopped = true;
    if (m_bLoggerEnabled) {
        WsjcppLog::info(TAG, "Stopped");
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebHttpThreadWorker::handle(WsjcppLightWebHttpRequest *pRequest, WsjcppLightWebHttpResponse *pResponse) {
    std::vector<WsjcppLightWebHttpHandlerBase *>::iterator it; 
    for (it = m_pVHandlers->begin(); it < m_pVHandlers->end(); it++) {
        WsjcppLightWebHttpHandlerBase *pHandler = *it;
        if (!pHandler->canHandle(m_sName, pRequest)) {
            continue;
        }
        if (!pHandler->handle(m_sName, pRequest, pResponse)) {
            if (m_bLoggerEnabled) {
                WsjcppLog::warn(TAG, pHandler->name() + " - could not handle request '" + pRequest->getRequestPath() + "'");
            }
            pResponse->internalServerError().sendEmpty();
        }
        return;
    }
    pResponse->notFound().sendEmpty();
}

// ----------------------------------------------------------------------
// WsjcppLightWebServer

WsjcppLightWebServer::WsjcppLightWebServer() {
    TAG = "WsjcppLightWebServer";
    m_nMaxWorkers = 4;
    m_pDeque = new WsjcppLightWebDequeHttpRequests();
    m_pVHandlers = new std::vector<WsjcppLightWebHttpHandlerBase *>();
    m_bStop = false;
    setPort(8080);
    setLoggerEnable(false);
    m_nBacklog = 10;
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::setPort(int nPort) {
    // TODO use a port validators
    if (nPort <= 10 || nPort > 65535) {
        WsjcppLog::throw_err(TAG, "Port must be 11...65535");
    }
    m_nPort = nPort;
    m_sPort = std::to_string(m_nPort);
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::setMaxWorkers(int nMaxWorkers) {
    // TODO number validator
    if (nMaxWorkers > 0 && nMaxWorkers <= 100) {
        m_nMaxWorkers = nMaxWorkers;
    } else {
        WsjcppLog::warn(TAG, "Max workers must be 1...100");
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::setLoggerEnable(bool bEnable) {
    m_bLoggerEnabled = bEnable;
    m_pDeque->setLoggerEnable(bEnable);
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::startSync() {

    m_nListenerSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nListenerSockFd <= 0) {
        WsjcppLog::err(TAG, "Failed to establish socket connection");
        return;
    }
    int enable = 1;
    if (setsockopt(m_nListenerSockFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        WsjcppLog::err(TAG, "setsockopt(SO_REUSEADDR) failed");
        return;
    }

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serverAddress.sin_port = htons(m_nPort);
    if (bind(m_nListenerSockFd, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress)) == -1) {
        WsjcppLog::err(TAG, "Error binding to port " + std::to_string(m_nPort));
        return;
    }
    listen(m_nListenerSockFd, m_nBacklog);
    // signal(SIGCHLD, SIG_IGN);
    WsjcppLog::info("LightHttpServer", "Light Http Server started on " + std::to_string(m_nPort) + " port.");

    std::string str;
    m_bStop = false;
    this->checkAndRestartWorkers();
    while (!m_bStop) {
        struct sockaddr_in clientAddress;
        socklen_t sosize  = sizeof(clientAddress);
        int nSockFd = accept(m_nListenerSockFd,(struct sockaddr*)&clientAddress,&sosize);
        std::string sAddress = inet_ntoa(clientAddress.sin_addr);
        if (m_bLoggerEnabled) {
            WsjcppLog::info(TAG, "Connected " + sAddress + ":" + std::to_string(clientAddress.sin_port));
        }
        int option_value = 1; /* Set NOSIGPIPE to ON */
        //if (setsockopt (nSockFd, SOL_SOCKET, SO_NOSIGPIPE, &option_value, sizeof (option_value)) < 0) {
            // perror ("setsockopt(,,SO_NOSIGPIPE)");
        //}

        WsjcppLightWebHttpRequest *pInfo = new WsjcppLightWebHttpRequest(nSockFd, sAddress);
        // info will be removed inside a thread
        m_pDeque->pushRequest(pInfo); // here will be unlocked workers
        // this->checkAndRestartWorkers();
        // TODO check workers if something stop - then restart
        // pthread_create(&m_serverThread, NULL, &newRequest, (void *)pInfo);
        // ???? accept must be lock this thread
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    m_pDeque->cleanup();
    this->stopAndRemoveWorkers();
    close(m_nListenerSockFd);
}

// 
// ----------------------------------------------------------------------

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// used this https://masandilov.ru/network/guide_to_network_programming7

int WsjcppLightWebServer::startSync2() {
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage clientAddress; // client address
    const int nBufferLength = 1024;
    char sBuffer[nBufferLength];    // buffer for client data
    int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, m_sPort.c_str(), &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        return 1;
	}
	
	for (p = ai; p != NULL; p = p->ai_next) {
    	m_nListenerSockFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (m_nListenerSockFd < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(m_nListenerSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(m_nListenerSockFd, p->ai_addr, p->ai_addrlen) < 0) {
			close(m_nListenerSockFd);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
        WsjcppLog::err(TAG, "Error binding to port " + std::to_string(m_nPort));
        return 2;
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(m_nListenerSockFd, m_nBacklog) == -1) {
        perror("listen");
        return 3;
    }

    // add the listener to the master set
    FD_SET(m_nListenerSockFd, &master);

    // keep track of the biggest file descriptor
    fdmax = m_nListenerSockFd; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            return 4;
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == m_nListenerSockFd) {
                    // handle new connections
                    socklen_t clientAddressLength = sizeof clientAddress;
					newfd = accept(m_nListenerSockFd, (struct sockaddr *)&clientAddress, &clientAddressLength);

					if (newfd == -1) {
                        WsjcppLog::err(TAG, "accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        logNewConnection(clientAddress, clientAddressLength);
                    }
                } else {
                    // handle data from a client
                    std::cout << i << std::endl;
                    if ((nbytes = recv(i, sBuffer, nBufferLength, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            eraseIncomeRequest(i);
                            WsjcppLog::err(TAG, "selectserver: socket " + std::to_string(i) + " hung up");
                        } else {
                            WsjcppLog::err(TAG, "recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        addIncomeRequest(i);
                        m_mapIncomeRequests[i]->appendRecieveRequest(sBuffer, nbytes);
                        if (m_mapIncomeRequests[i]->isEnoughAppendReceived()) {
                            // TODO put to qeque requets and erase from m_mapIncomeRequests
                        }

                        WsjcppLog::err(TAG, "recved " + std::string(sBuffer, nbytes));
                        // std::cout << std::string(sBuffer) << std::endl;

                        // we got some data from a client
                        for (j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != m_nListenerSockFd && j != i) {
                                    if (send(j, sBuffer, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    return 0;
}
// ----------------------------------------------------------------------

void* wsjcppLightWebServerProcessServerStart(void *arg) {
    WsjcppLightWebServer *pLightWebServer = (WsjcppLightWebServer *)arg;
    pthread_detach(pthread_self());
    pLightWebServer->startSync();
    return 0;
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::start() {
    m_bStop = false;
    pthread_create(&m_serverThread, NULL, &wsjcppLightWebServerProcessServerStart, (void *)this);
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::stop() {
    m_bStop = true;
} 

// ----------------------------------------------------------------------

void WsjcppLightWebServer::addHandler(WsjcppLightWebHttpHandlerBase *pHandler) {
    m_pVHandlers->push_back(pHandler);
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::checkAndRestartWorkers() {
    if (!m_bStop) {
        if (m_vWorkers.size() < m_nMaxWorkers) {
            int nSize = m_vWorkers.size();
            for (int i = nSize; i < m_nMaxWorkers; i++) {
                m_vWorkers.push_back(
                    new WsjcppLightWebHttpThreadWorker(
                        "worker" + std::to_string(i), 
                        m_pDeque, 
                        m_pVHandlers,
                        m_bLoggerEnabled
                    )
                );
            }
        }

        for (int i = 0; i < m_vWorkers.size(); i++) {
            WsjcppLightWebHttpThreadWorker *pWorker = m_vWorkers[i];
            pWorker->start();
        }
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::stopAndRemoveWorkers() {
    if (m_bStop) {
        for (int i = 0; i < m_vWorkers.size(); i++) {
            m_vWorkers[i]->stop();
            delete m_vWorkers[i];
        }
        m_vWorkers.clear();
    }
}

// ----------------------------------------------------------------------

std::string WsjcppLightWebServer::readAddress(int nSockFd) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(nSockFd, (struct sockaddr *)&addr, &addr_size);
    char *clientip = new char[20];
    memset(clientip, 0, 20);
    strcpy(clientip, inet_ntoa(addr.sin_addr));
    return std::string(clientip);
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::logNewConnection(sockaddr_storage &clientAddress, socklen_t &clientAddressLength) {
    char hoststr[NI_MAXHOST];
    char portstr[NI_MAXSERV];

    int rc = getnameinfo(
        (struct sockaddr *)&clientAddress, 
        clientAddressLength,
        hoststr,
        sizeof(hoststr),
        portstr,
        sizeof(portstr), 
        NI_NUMERICHOST | NI_NUMERICSERV
    );

    if (rc == 0) {
        if (m_bLoggerEnabled) {
            WsjcppLog::info(TAG, "New connection from " + std::string(hoststr) + " " + std::string(portstr));
        }
    } else {
        WsjcppLog::err(TAG, "Failed call getnameinfo by address");
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::addIncomeRequest(int i) {
    std::map<int, WsjcppLightWebHttpRequest *>::iterator it = m_mapIncomeRequests.find(i);
    if (it == m_mapIncomeRequests.end()) { // create a new one
        // TODO set address
        // std::string sAddress = inet_ntoa(clientAddress.sin_addr);
        m_mapIncomeRequests[i] = new WsjcppLightWebHttpRequest(i, "");
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebServer::eraseIncomeRequest(int i) {
    std::map<int, WsjcppLightWebHttpRequest *>::iterator it = m_mapIncomeRequests.find(i);
    if (it != m_mapIncomeRequests.end()) { // create a new one
        m_mapIncomeRequests.erase(it);
    }
}