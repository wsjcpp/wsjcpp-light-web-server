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
// WSJCppLightWebHttpRequest

WSJCppLightWebHttpRequest::WSJCppLightWebHttpRequest(int nSockFd, const std::string &sAddress) {
    TAG = "WSJCppLightWebHttpRequest";
    m_sUniqueId = WSJCppCore::createUuid();
    m_nSockFd = nSockFd;
    m_sAddress = sAddress;
    m_bClosed = false;
    m_sRequest = "";
    m_nParserState = EnumParserState::START;
    long nSec = WSJCppCore::currentTime_seconds();
    m_sLastModified = WSJCppCore::formatTimeForWeb(nSec);
    m_nContentLength = 0;
}

// ----------------------------------------------------------------------

int WSJCppLightWebHttpRequest::getSockFd() {
    return m_nSockFd;
}

// ----------------------------------------------------------------------

std::string WSJCppLightWebHttpRequest::getUniqueId() {
    return m_sUniqueId;
}

// ----------------------------------------------------------------------

std::string WSJCppLightWebHttpRequest::requestType() {
    return m_sRequestType;
}

// ----------------------------------------------------------------------

std::string WSJCppLightWebHttpRequest::requestPath() {
    return m_sRequestPath;
}

// ----------------------------------------------------------------------

std::string WSJCppLightWebHttpRequest::requestHttpVersion() {
    return m_sRequestHttpVersion;
}

// ----------------------------------------------------------------------
// TODO redesign this to vector
std::map<std::string,std::string> &WSJCppLightWebHttpRequest::requestQueryParams() {
    return m_sRequestQueryParams;
}

// ----------------------------------------------------------------------

std::string WSJCppLightWebHttpRequest::getAddress() {
    return m_sAddress;
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpRequest::appendRecieveRequest(const std::string &sRequestPart) {
    m_sRequest += sRequestPart;
    const std::string sContentLengthPrefix = "content-length:";
    if (m_nParserState == EnumParserState::START) {
        m_vHeaders.clear();
        // WSJCppLog::info(TAG, "START \n>>>\n" + m_sRequest + "\n<<<\n");

        std::istringstream f(m_sRequest);
        std::string sLine = "";
        int nSize = 0;
        bool bHeadersEnded = false;
        while (getline(f, sLine, '\n')) {
            nSize += sLine.length() + 1;
            WSJCppCore::trim(sLine);
            // WSJCppLog::info(TAG, "Line: {" + sLine + "}, size=" + std::to_string(sLine.length()));
            if (sLine.length() == 0) {
                bHeadersEnded = true;
                break;
            }
            m_vHeaders.push_back(sLine);

            WSJCppCore::to_lower(sLine);
            if (!sLine.compare(0, sContentLengthPrefix.size(), sContentLengthPrefix)) {
                m_nContentLength = atoi(sLine.substr(sContentLengthPrefix.size()).c_str());
                // WSJCppLog::warn(TAG, "Content-Length: " + std::to_string(m_nContentLength));
            }
        }

        if (bHeadersEnded) {
            if (m_vHeaders.size() > 0) {
                this->parseFirstLine(m_vHeaders[0]);
            }
            m_sRequest.erase(0, nSize);
            // WSJCppLog::info(TAG, "AFTER ERASE \n>>>\n" + m_sRequest + "\n<<<\n");
            m_nParserState = EnumParserState::BODY;
        } else {
            // WSJCppLog::info(TAG, "Not ended");
        }
    }
    
    if (m_nParserState == EnumParserState::BODY && m_sRequest.length() == m_nContentLength) {
        m_nParserState = EnumParserState::ENDED;
        m_sRequestBody = m_sRequest;
    }
}

// ----------------------------------------------------------------------

bool WSJCppLightWebHttpRequest::isEnoughAppendReceived() {
    return m_nParserState == EnumParserState::ENDED;
}

// ----------------------------------------------------------------------

void WSJCppLightWebHttpRequest::parseFirstLine(const std::string &sHeader) {
    if (sHeader.size() > 0) {
        std::istringstream f(sHeader);
        std::vector<std::string> params;
        std::string s;
        while (getline(f, s, ' ')) {
            params.push_back(s);
        }
        if (params.size() > 0) {
            m_sRequestType = params[0];
        }

        if (params.size() > 1) {
            m_sRequestPath = params[1];
        }

        // TODO m_sRequestPath - need split by ? if exists
        if (params.size() > 2) {
            m_sRequestHttpVersion = params[2];
        }
    }
    // TODO process the path '/1/../2/' to /2/ - vuln
    // curl --path-as-is "http://localhost:1234/1/../2/"

    // parse query
    std::size_t nFound = m_sRequestPath.find("?");
      if (nFound != std::string::npos) {
        std::string sQuery = m_sRequestPath.substr(nFound+1);
        m_sRequestPath = m_sRequestPath.substr(0, nFound);
        std::istringstream f(sQuery);
        std::string sParam;
        while (getline(f, sParam, '&')) {
            std::size_t nFound2 = sParam.find("=");
            std::string sValue = sParam.substr(nFound2+1);
            std::string sName = sParam.substr(0, nFound2);
            m_sRequestQueryParams[sName] = sValue; // wrong use map for params
        }
    }
}
