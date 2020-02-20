cmake_minimum_required(VERSION 3.0)

project(wsjcpp-light-web-server C CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

set(CMAKE_CXX_STANDARD 11)
set(EXECUTABLE_OUTPUT_PATH ${wsjcpp-light-web-server_SOURCE_DIR})

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")

list (APPEND WSJCPP_SOURCES "src/main.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_request.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_request.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_response.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_response.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_deque_http_requests.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_deque_http_requests.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_handler_rewrite_folder.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_handler_rewrite_folder.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_handler_web_folder.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_http_handler_web_folder.cpp")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_server.h")
list (APPEND WSJCPP_SOURCES "src/wsjcpp_light_web_server.cpp")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (wsjcpp-light-web-server ${WSJCPP_SOURCES})

target_link_libraries(wsjcpp-light-web-server ${WSJCPP_LIBRARIES})

install(
    TARGETS
        wsjcpp-light-web-server
    RUNTIME DESTINATION
        /usr/bin
)

