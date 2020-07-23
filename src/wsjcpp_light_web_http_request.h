#ifndef WSJCPP_LIGHT_WEB_HTTP_REQUEST_H
#define WSJCPP_LIGHT_WEB_HTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>

// ---------------------------------------------------------------------

class WsjcppLightWebHttpRequestQueryValue {
    public:
        WsjcppLightWebHttpRequestQueryValue(const std::string &sName, const std::string &sValue);
        std::string getName() const;
        std::string getValue() const;
    private:
        std::string m_sName;
        std::string m_sValue;
};

// ---------------------------------------------------------------------

class WsjcppLightWebHttpRequest {
    public:
        WsjcppLightWebHttpRequest(
            int nSockFd,
            const std::string &sAddress
        );
        ~WsjcppLightWebHttpRequest() {};

        int getSockFd() const;
        std::string getUniqueId() const;
        void appendRecieveRequest(const std::string &sRequestPart); // depraceted
        bool appendRecieveRequest(const char *sRequestPart, int nLength);
        bool isEnoughAppendReceived() const;
        
        std::string getAddress() const;
        const std::string &getRequestData() const;
        std::string getRequestType() const;
        std::string getRequestPath() const;
        std::string getRequestBody() const;
        std::string getRequestHttpVersion() const;
        const std::string &getHeaderConnection() const; 
        const std::vector<WsjcppLightWebHttpRequestQueryValue> &getRequestQueryParams();

    private:
        std::string TAG;

        void parseFirstLine(const std::string &sHeader);
        int parseRequestType(int nPos, const char *sRequestPart, int nLength);
        int parseRequestPathAndGetParams(int nPos, const char *sRequestPart, int nLength);
        int parseRequestHttpVersion(int nPos, const char *sRequestPart, int nLength);
        int parseRequestNextHeader(int nPos, const char *sRequestPart, int nLength);

        enum EnumParserState {
            START,
            HEADERS,
            BODY,
            ENDED
        };
        int m_nSockFd;
        bool m_bClosed;
        EnumParserState m_nParserState;
        std::vector<std::string> m_vHeaders;
        int m_nHeaderContentLength;
        std::string m_sHeaderConnection;
        std::string m_sUniqueId;
        std::string m_sRequest;
        std::string m_sAddress;
        std::string m_sRequestType;
        std::string m_sRequestPath;
        std::string m_sRequestPathAndGetParams;
        std::string m_sRequestBody;
        char *m_bRequestBody;
        int m_nRequestBodyWritePosition;
        std::vector<WsjcppLightWebHttpRequestQueryValue> m_vRequestQueryParams;
        std::string m_sRequestHttpVersion;
};

#endif // WSJCPP_LIGHT_WEB_HTTP_REQUEST_H


