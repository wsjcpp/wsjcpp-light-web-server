#include <wsjcpp_core.h>
#include <wsjcpp_light_web_server.h>
#include "wsjcpp_light_web_http_handler_rewrite_folder.h"
#include "wsjcpp_light_web_http_handler_web_folder.h"

int main(int argc, const char* argv[]) {
    std::string TAG = "MAIN";
    std::string appName = std::string(WSJCPP_NAME);
    std::string appVersion = std::string(WSJCPP_VERSION);
    if (!WSJCppCore::dirExists(".logs")) {
        WSJCppCore::makeDir(".logs");
    }
    WSJCppLog::setPrefixLogFile("wsjcpp");
    WSJCppLog::setLogDirectory(".logs");

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

    WSJCppLightWebServer httpServer;
    httpServer.setPort(1234);
    httpServer.setMaxWorkers(1);
    if (sType == "folder") {
        httpServer.addHandler((WSJCppLightWebHttpHandlerBase *)new WSJCppLightWebHttpHandlerWebFolder("/app/", sDir));
        httpServer.addHandler((WSJCppLightWebHttpHandlerBase *)new WSJCppLightWebHttpHandlerWebFolder("/", sDir));
    } else if (sType == "rewrite") {
        httpServer.addHandler((WSJCppLightWebHttpHandlerBase *)new WSJCppLightWebHttpHandlerRewriteFolder("/app/", sDir));
        httpServer.addHandler((WSJCppLightWebHttpHandlerBase *)new WSJCppLightWebHttpHandlerRewriteFolder("/", sDir));
    }
    httpServer.startSync();
    return 0;
}

