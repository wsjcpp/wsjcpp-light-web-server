#include <wsjcpp_core.h>
#include <wsjcpp_light_web_server.h>
#include "wsjcpp_light_web_http_handler_rewrite_folder.h"
#include "wsjcpp_light_web_http_handler_web_folder.h"

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_NAME);
    std::string appVersion = std::string(WSJCPP_VERSION);
    if (!WsjcppCore::dirExists(".logs")) {
        WsjcppCore::makeDir(".logs");
    }
    WsjcppLog::setPrefixLogFile("wsjcpp");
    WsjcppLog::setLogDirectory(".logs");
    std::string sUsage = "Usage: " + std::string(argv[0]) + " [folder|rewrite] <dir> <port>";

    if (argc != 4) {
        std::cout << sUsage << std::endl;
        return -1;
    }
    std::string sType = std::string(argv[1]);
    if (sType != "folder" && sType != "rewrite") {
        std::cout << sUsage << std::endl;
        return -1;
    }

    std::string sDir = std::string(argv[2]);
    std::string sPort = std::string(argv[3]);
    int nPort = std::atoi(sPort.c_str());
    if (nPort < 10) {
        std::cout << "Please set port more then 0" << std::endl;
        return -1;
    }

    if (nPort >= 65536) {
        std::cout << "Please set port less then 65536" << std::endl;
        return -1;
    }

    WsjcppLightWebServer httpServer;
    httpServer.setPort(nPort);
    httpServer.setMaxWorkers(4);
    // httpServer.setLoggerEnable(true);
    httpServer.setLoggerEnable(false);
    if (sType == "folder") {
        httpServer.addHandler(new WsjcppLightWebHttpHandlerWebFolder("/app/", sDir));
        httpServer.addHandler(new WsjcppLightWebHttpHandlerWebFolder("/", sDir));
    } else if (sType == "rewrite") {
        httpServer.addHandler(new WsjcppLightWebHttpHandlerRewriteFolder("/app/", sDir));
        httpServer.addHandler(new WsjcppLightWebHttpHandlerRewriteFolder("/", sDir));
    }
    httpServer.startSync();
    return 0;
}

