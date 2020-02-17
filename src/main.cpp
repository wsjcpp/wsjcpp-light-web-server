#include <string.h>
#include <iostream>
#include <algorithm>
#include <wsjcpp_core.h>
#include <wsjcpp_light_web_server.h>

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_NAME);
    std::string appVersion = std::string(WSJCPP_VERSION);
    if (!WSJCppCore::dirExists(".logs")) {
        WSJCppCore::makeDir(".logs");
    }
    WSJCppLog::setPrefixLogFile("wsjcpp");
    WSJCppLog::setLogDirectory(".logs");

    LightHttpServer server;
    server.setPort(1234);
    server.setMaxWorkers(1);
    server.startSync();
    return 0;
}

