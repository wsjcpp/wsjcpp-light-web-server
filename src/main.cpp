#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include <wsjcpp_light_web_server.h>
#include "http_handlers/http_handler_web_folder_example.h"

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_NAME);
    std::string appVersion = std::string(WSJCPP_VERSION);
    if (!WSJCppCore::dirExists(".logs")) {
        WSJCppCore::makeDir(".logs");
    }
    WSJCppLog::setPrefixLogFile("wsjcpp");
    WSJCppLog::setLogDirectory(".logs");

    LightHttpServer httpServer;
    httpServer.setPort(1234);
    httpServer.setMaxWorkers(1);
    httpServer.handlers()->add((WSJCppLightWebHttpHandlerBase *)new HttpHandlerWebFolderExample("./web"));


    httpServer.startSync();
    return 0;
}

