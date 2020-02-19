#include "unit_test_parse_http_request.h"
#include <vector>
#include <wsjcpp_core.h>
#include <wsjcpp_light_web_http_request.h>

REGISTRY_UNIT_TEST(UnitTestParseHttpRequest)

UnitTestParseHttpRequest::UnitTestParseHttpRequest()
    : WSJCppUnitTestBase("UnitTestParseHttpRequest") {
}

// ---------------------------------------------------------------------

void UnitTestParseHttpRequest::init() {
    // nothing
}

// ---------------------------------------------------------------------

bool UnitTestParseHttpRequest::run() {
    bool bTestSuccess = true;
    struct LPartsOfRequest {
        LPartsOfRequest(std::string sPart, bool enough) {
            this->sPart = sPart;
            this->enough = enough;
        }
        std::string sPart;
        bool enough;
    };

    struct LTest {
        LTest(
            int sockFd, 
            std::string address, 
            std::vector<LPartsOfRequest> parts
        ) {
            this->sockFd = sockFd;
            this->address = address;
            this->parts = parts;
        }
        int sockFd;
        std::string address;
        std::vector<LPartsOfRequest> parts;
    };

    std::vector<LTest> tests;
    tests.push_back(LTest(0, "some-address", { 
        LPartsOfRequest("GET /pub/WWW/TheProject.html HTTP/1.1\n", false), 
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("Content-Length: 0\n", false),
        LPartsOfRequest("\n", true)
    }));

    tests.push_back(LTest(1, "some-address2", { 
        LPartsOfRequest("GET /pub/WWW/TheProject.html HTTP/1.1\n", false),
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("Content-Length: 1\n", false),
        LPartsOfRequest("\n", false),
        LPartsOfRequest("1", true)
    }));

    for (int i = 0; i < tests.size(); i++) {
        LTest test = tests[i];
        WSJCppLightWebHttpRequest request(test.sockFd, test.address);
        std::string sNTest = "test" + std::to_string(i) + "#";

        compareS(bTestSuccess, sNTest + " request address", request.getAddress(), test.address);
        compareN(bTestSuccess, sNTest + " request sockfd", request.getSockFd(), test.sockFd);

        for (int n = 0; n < test.parts.size(); n++) {
            std::string sNTest2 = sNTest + " append part" + std::to_string(n) + "#";
            request.appendRecieveRequest(test.parts[n].sPart);
            compareB(bTestSuccess, sNTest2, request.isEnoughAppendReceived(), test.parts[n].enough);
        }
    }
    return bTestSuccess;
}

