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

    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " [folder|rewrite] <dir>" << std::endl;
        return -1;
    }
    std::string sType = std::string(argv[1]);
    if (sType != "folder" && sType != "rewrite") {
        std::cout << "Usage: " << argv[0] << " [folder|rewrite] <dir>" << std::endl;
        return -1;
    }

    std::string sDir = std::string(argv[2]);

    WsjcppLightWebServer httpServer;
    httpServer.setPort(1234);
    httpServer.setMaxWorkers(4);
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

