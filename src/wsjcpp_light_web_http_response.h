#ifndef WSJCPP_LIGHT_WEB_HTTP_RESPONSE_H
#define WSJCPP_LIGHT_WEB_HTTP_RESPONSE_H

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

class WSJCppLightWebHttpResponse {
    public:
        static std::map<int, std::string> *g_mapReponseDescription;
        
        WSJCppLightWebHttpResponse(int nSockFd);

        WSJCppLightWebHttpResponse &ok();
        WSJCppLightWebHttpResponse &badRequest();
        WSJCppLightWebHttpResponse &forbidden();
        WSJCppLightWebHttpResponse &notFound();
        WSJCppLightWebHttpResponse &payloadTooLarge();
        WSJCppLightWebHttpResponse &internalServerError();
        WSJCppLightWebHttpResponse &notImplemented();
        WSJCppLightWebHttpResponse &requestTimeout();

        WSJCppLightWebHttpResponse &noCache();
        WSJCppLightWebHttpResponse &cacheSec(int nCacheSec);

        void sendText(const std::string &sBody);
        void sendEmpty();
        void sendOptions(const std::string &sOptions);
        void sendFile(const std::string &sFilePath);
        void sendBuffer(const std::string &sFilePath, const char *pBuffer, const int nBufferSize);

    private:
        std::string prepareHeaders(int nLength);
        std::string detectTypeOfFile(const std::string &sFilePath);

        std::string TAG;

        int m_nSockFd;
        bool m_bClosed;
        int m_nResponseCode;
        std::string m_sDataType;
        std::string m_sCacheControl;
        std::string m_sLastModified;
};

#endif // WSJCPP_LIGHT_WEB_HTTP_RESPONSE_H


