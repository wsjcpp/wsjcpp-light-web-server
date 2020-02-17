#ifndef WSJCPP_LIGHT_WEB_HTTP_REQUEST_H
#define WSJCPP_LIGHT_WEB_HTTP_REQUEST_H

#include "wsjcpp_light_web_http_request.h"

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

// ---------------------------------------------------------------------

class WSJCppLightWebHttpRequest {
    public:
        WSJCppLightWebHttpRequest(
            int nSockFd,
            const std::string &sAddress
        );
        ~WSJCppLightWebHttpRequest() {};

        int sockFd();
        void appendRecieveRequest(const std::string &sRequestPart);
        bool isEnoughAppendReceived();
        
        std::string address();
        std::string requestType();
        std::string requestPath();
        std::string requestBody();
        std::string requestHttpVersion();
        std::map<std::string,std::string> &requestQueryParams();

    private:
        std::string TAG;

        void parseFirstLine(const std::string &sHeader);

        enum EnumParserState {
            START,
            BODY,
            ENDED
        };
        int m_nSockFd;
        bool m_bClosed;
        EnumParserState m_nParserState;
        std::vector<std::string> m_vHeaders;
        int m_nContentLength;
        std::string m_sRequest;
        std::string m_sAddress;
        std::string m_sRequestType;
        std::string m_sRequestPath;
        std::string m_sRequestBody;
        std::map<std::string,std::string> m_sRequestQueryParams; // wrong use map for params
        std::string m_sRequestHttpVersion;

        std::string m_sResponseCacheControl;
        std::string m_sLastModified;
};

#endif // WSJCPP_LIGHT_WEB_HTTP_REQUEST_H


