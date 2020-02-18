# wsjcpp-light-web-server

[![Build Status](https://api.travis-ci.org/wsjcpp/wsjcpp-light-web-server.svg?branch=master)](https://travis-ci.org/wsjcpp/wsjcpp-light-web-server) [![Github Stars](https://img.shields.io/github/stars/wsjcpp/wsjcpp-light-web-server.svg?label=github%20%E2%98%85)](https://github.com/wsjcpp/wsjcpp-light-web-server/stargazers) [![Github Stars](https://img.shields.io/github/contributors/wsjcpp/wsjcpp-light-web-server.svg)](https://github.com/wsjcpp/wsjcpp-light-web-server/) [![Github Forks](https://img.shields.io/github/forks/wsjcpp/wsjcpp-light-web-server.svg?label=github%20forks)](https://github.com/wsjcpp/wsjcpp-light-web-server/network/members)

## Integrate to your project

Just include this files:

- src/wsjcpp_core/wsjcpp_core.h
- src/wsjcpp_core/wsjcpp_core.cpp
- src/wsjcpp_light_web_http_request.h
- src/wsjcpp_light_web_http_request.cpp
- src/wsjcpp_light_web_http_response.h
- src/wsjcpp_light_web_http_response.cpp
- src/wsjcpp_light_web_server.h
- src/wsjcpp_light_web_server.cpp


## Integrate via wsjcpp

```
$ wsjcpp install https://github.com/wsjcpp/wsjcpp-light-web-server:master
```

# Http Handlers examples

Example init, add handler and start server
```
LightHttpServer httpServer;
httpServer.setPort(1234);
httpServer.setMaxWorkers(1);
httpServer.handlers()->add((WSJCppLightWebHttpHandlerBase *)new HttpHandlerWebFolderExample("./web"));
httpServer.startSync(); // this method will be hold current thread, if you with you can call just start/stop command
```

## Web Folder example

Define class:

```
#include <wsjcpp_light_web_server.h>

class HttpHandlerWebFolderExample : WSJCppLightWebHttpHandlerBase {
    public:
        HttpHandlerWebFolderExample(const std::string &sWebFolder);
        virtual bool canHandle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest);
        virtual bool handle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest);

    private:
        std::string TAG;
        std::string m_sWebFolder;
};
```

Class implementation:
```
#include <wsjcpp_core.h>

// ----------------------------------------------------------------------

HttpHandlerWebFolderExample::HttpHandlerWebFolderExample(const std::string &sWebFolder)
    : WSJCppLightWebHttpHandlerBase("web-folder") {

    TAG = "HttpHandlerWebFolderExample";
    m_sWebFolder = sWebFolder;
}

// ----------------------------------------------------------------------

bool HttpHandlerWebFolderExample::canHandle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest) {
    std::string _tag = TAG + "-" + sWorkerId;
    // WSJCppLog::warn(_tag, "canHandle: " + pRequest->requestPath());
    std::string sRequestPath = pRequest->requestPath();
    
    if (sRequestPath == "") {
        sRequestPath = "/";
        WSJCppLog::warn(_tag, "Request path is empty");
    }

    if (sRequestPath == "/") {
        sRequestPath = "/index.html";
    }

    if (!WSJCppCore::dirExists(m_sWebFolder)) {
        WSJCppLog::warn(_tag, "Directory " + m_sWebFolder + " does not exists");
    }
    return true;
}

// ----------------------------------------------------------------------

bool HttpHandlerWebFolderExample::handle(const std::string &sWorkerId, WSJCppLightWebHttpRequest *pRequest) {
    std::string _tag = TAG + "-" + sWorkerId;
    std::string sRequestPath = pRequest->requestPath();
    // WSJCppLog::warn(_tag, pRequest->requestPath());
    
    if (sRequestPath == "") {
        sRequestPath = "/";
    }
    
    std::string sFilePath = m_sWebFolder + sRequestPath; // TODO check /../ in path
    if (WSJCppCore::fileExists(sFilePath)) {
        WSJCppLightWebHttpResponse resp(pRequest->sockFd());
        resp.cacheSec(60).ok().sendFile(sFilePath);
    } else {
        std::string sFilePath = m_sWebFolder + "/index.html";
        WSJCppLightWebHttpResponse resp(pRequest->sockFd());
        resp.cacheSec(60).ok().sendFile(sFilePath);    
    }
    return true;
}
```