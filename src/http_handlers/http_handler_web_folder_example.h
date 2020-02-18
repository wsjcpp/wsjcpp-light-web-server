#ifndef HTTP_HANDLER_WEB_FOLDER_EXAMPLE_H
#define HTTP_HANDLER_WEB_FOLDER_EXAMPLE_H

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

#endif // HTTP_HANDLER_WEB_FOLDER_EXAMPLE_H