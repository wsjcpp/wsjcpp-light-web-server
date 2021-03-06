# wsjcpp-light-web-server

[![Build Status](https://api.travis-ci.com/wsjcpp/wsjcpp-light-web-server.svg?branch=master)](https://travis-ci.com/wsjcpp/wsjcpp-light-web-server) [![Github Stars](https://img.shields.io/github/stars/wsjcpp/wsjcpp-light-web-server.svg?label=github%20%E2%98%85)](https://github.com/wsjcpp/wsjcpp-light-web-server/stargazers) [![Github Stars](https://img.shields.io/github/contributors/wsjcpp/wsjcpp-light-web-server.svg)](https://github.com/wsjcpp/wsjcpp-light-web-server/) [![Github Forks](https://img.shields.io/github/forks/wsjcpp/wsjcpp-light-web-server.svg?label=github%20forks)](https://github.com/wsjcpp/wsjcpp-light-web-server/network/members)

## Integrate to your project

Include this files:

- src.wsjcpp/wsjcpp_core/wsjcpp_core.h
- src.wsjcpp/wsjcpp_core/wsjcpp_core.cpp
- src.wsjcpp/nlohmann_json/json.hpp
- src/wsjcpp_light_web_http_request.h
- src/wsjcpp_light_web_http_request.cpp
- src/wsjcpp_light_web_http_response.h
- src/wsjcpp_light_web_http_response.cpp
- src/wsjcpp_light_web_http_handler_rewrite_folder.h
- src/wsjcpp_light_web_http_handler_rewrite_folder.cpp
- src/wsjcpp_light_web_http_handler_web_folder.h
- src/wsjcpp_light_web_http_handler_web_folder.cpp
- src/wsjcpp_light_web_server.h
- src/wsjcpp_light_web_server.cpp

## Integrate via wsjcpp

```
$ wsjcpp install https://github.com/wsjcpp/wsjcpp-light-web-server:master
```

Fast generate handler:

```
$ wsjcpp generate WsjcppLightWebHttpHandler SomePage
```
So will be generated sample class 'LightWebHttpHandlerSomePage' with TODO

## Examples

### Example for simular rewrute_rules

Simpular apache:
```
RewriteCond %{REQUEST_FILENAME} !-f
RewriteCond %{REQUEST_FILENAME} !-d
RewriteRule . index.html
```

Contains base handler:
```
WsjcppLightWebHttpHandlerRewriteFolder(sPrefixPath, sDirPath)
```
Where
* sPrefixPath - like "/app1/" -> "http://localhost:1234/app1/"
* sDirPath - relative or absulte path to folder with web files

Specific: if file will be not found in web folder so will be returned file index.html

Will be good for single-web-app (like angular)

Example init, add handler and start server
```
#include <wsjcpp_light_web_http_handler_rewrite_folder.h>

... 
WsjcppLightWebServer httpServer;
httpServer.setPort(1234);
httpServer.setMaxWorkers(1);
httpServer.addHandler((WsjcppLightWebHttpHandlerBase *)new WsjcppLightWebHttpHandlerRewriteFolder("/app1/", "./web"));
httpServer.startSync(); // this method will be hold current thread, if you with you can call just start/stop command
```

After compile and start will be available on `http://localhost:1234/app1/`

### Example simple web folder

Contains base handler:
```
WsjcppLightWebHttpHandlerWebFolder(sPrefixPath, sDirPath)
```

Where
* sPrefixPath - like "/app2/" -> "http://localhost:1234/app2/"
* sDirPath - relative or absulte path to folder with web files

Specific: if file does not exists wil be returned 404 not found

Example init, add handler and start server
```
#include <wsjcpp_light_web_http_handler_web_folder.h>

... 
WsjcppLightWebServer httpServer;
httpServer.setPort(1234);
httpServer.setMaxWorkers(1);
httpServer.addHandler((WsjcppLightWebHttpHandlerBase *)new WsjcppLightWebHttpHandlerWebFolder("/app2/", "./web"));
httpServer.startSync(); // this method will be hold current thread, if you with you can call just start/stop command
```

After compile and start will be available on `http://localhost:1234/app2/`

### Example custom handler

Define class:

header-file `http_handler_custom.h`:
```
#ifndef HTTP_HANDLER_CUSTOM_H
#define HTTP_HANDLER_CUSTOM_H

#include <wsjcpp_light_web_server.h>

class HttpHandlerCustom : WsjcppLightWebHttpHandlerBase {
    public:
        HttpHandlerCustom();
        virtual bool canHandle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest);
        virtual bool handle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest);

    private:
        std::string TAG;
};
#endif // HTTP_HANDLER_CUSTOM_H
```

source-file `http_handler_custom.cpp`:
```
#include <wsjcpp_core.h>
#include "http_handler_custom.h"

// ----------------------------------------------------------------------

HttpHandlerCustom::HttpHandlerCustom()
: WsjcppLightWebHttpHandlerBase("custom") {
    TAG = "HttpHandlerCustom";
}

// ----------------------------------------------------------------------

bool HttpHandlerCustom::canHandle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest) {
    std::string _tag = TAG + "-" + sWorkerId;
    std::string sRequestPath = pRequest->getRequestPath();

    if (sRequestPath == "/custom/" || sRequestPath == "/custom") {
        return true;    
    }
    if (sRequestPath == "/custom1/" || sRequestPath == "/custom1") {
        return true;    
    }
    return false;
}

// ----------------------------------------------------------------------

bool HttpHandlerCustom::handle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest) {
    std::string _tag = TAG + "-" + sWorkerId;
    std::string sRequestPath = pRequest->getRequestPath();
    // WsjcppLog::warn(_tag, sRequestPath);
    
    WsjcppLightWebHttpResponse resp(pRequest->getSockFd());
    if (sRequestPath == "/custom" || sRequestPath == "/custom/") {
        resp.cacheSec(60).ok().sendText(
            "<h1>This is custom</h1>"
        );
    } else if (sRequestPath == "/custom1" || sRequestPath == "/custom1/") {
        resp.cacheSec(60).ok().sendText(
            "<h1>But this is custom1</h1>"
        );
    } else {
        resp.noCache().ok().sendText(
            "<h1>Unknown</h1>"
        );
    }
    return true;
}
```

Example init, add handler and start server
__order is important! Server will call canHandle & handle in same order as addHandler called__
```
WsjcppLightWebServer httpServer;
httpServer.setPort(1234);
httpServer.setMaxWorkers(1);
httpServer.addHandler((WsjcppLightWebHttpHandlerBase *)new HttpHandlerCustom());
httpServer.addHandler((WsjcppLightWebHttpHandlerBase *)new WsjcppLightWebHttpHandlerRewriteFolder("/", "./web"));
httpServer.startSync(); // this method will be hold current thread, if you with you can call just start/stop command
```
