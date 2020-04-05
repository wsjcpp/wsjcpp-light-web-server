#include "unit_test_parse_http_request.h"
#include <vector>
#include <wsjcpp_core.h>
#include <wsjcpp_light_web_http_request.h>

REGISTRY_WSJCPP_UNIT_TEST(UnitTestParseHttpRequest)

UnitTestParseHttpRequest::UnitTestParseHttpRequest()
    : WsjcppUnitTestBase("UnitTestParseHttpRequest") {
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

    struct LPartsOfRequestQueryParam {
        LPartsOfRequestQueryParam(std::string sName, std::string sValue) {
            this->sName = sName;
            this->sValue = sValue;
        }
        std::string sName;
        std::string sValue;
    };

    struct LTest {
        LTest(
            int sockFd, 
            std::string address, 
            std::vector<LPartsOfRequest> parts,
            std::string expectedPath,
            std::string expectedType,
            std::string expectedBody,
            std::string expectedHttpVersion,
            std::vector<LPartsOfRequestQueryParam> expectedQueryParams
        ) {
            this->sockFd = sockFd;
            this->address = address;
            this->parts = parts;
            this->expectedPath = expectedPath;
            this->expectedType = expectedType;
            this->expectedBody = expectedBody;
            this->expectedHttpVersion = expectedHttpVersion;
            this->expectedQueryParams = expectedQueryParams;
        }
        int sockFd;
        std::string address;
        std::vector<LPartsOfRequest> parts;
        std::string expectedPath;
        std::string expectedType;
        std::string expectedBody;
        std::string expectedHttpVersion;
        std::vector<LPartsOfRequestQueryParam> expectedQueryParams;
    };

    std::vector<LTest> tests;
    tests.push_back(LTest(0, "some-address", { 
        LPartsOfRequest("GET /pub/WWW/TheProject.html HTTP/1.1\n", false), 
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("\n", true)
    }, "/pub/WWW/TheProject.html", "GET", "", "HTTP/1.1", {}));

    tests.push_back(LTest(1, "some-address2", { 
        LPartsOfRequest("GET /pub/WWW/TheProject.html HTTP/1.1\n", false),
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("Content-Length: 1\n", false),
        LPartsOfRequest("\n", false),
        LPartsOfRequest("1", true)
    }, "/pub/WWW/TheProject.html", "GET", "1", "HTTP/1.1", {}));

    tests.push_back(LTest(2, "some-address3", { 
        LPartsOfRequest("GET /1/../2 HTTP/1.1\n", false),
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("Content-Length: 1\n", false),
        LPartsOfRequest("\n", false),
        LPartsOfRequest("1", true)
    }, "/2", "GET", "1", "HTTP/1.1", {}));

    tests.push_back(LTest(3, "some-address4", { 
        LPartsOfRequest("POST /1/../../2/ HTTP/1.1\n", false),
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("Content-Length: 10\n", false),
        LPartsOfRequest("\n", false),
        LPartsOfRequest("{\"some\":1}   ", true)
    }, "/2/", "POST", "{\"some\":1}", "HTTP/1.1", {}));

    tests.push_back(LTest(4, "some-address4", { 
        LPartsOfRequest("GET /query?somebook=Hello%20sss&somebook=Hello%20ddd&somebook2=dmsf&p4=11_-%25dk123 HTTP/1.1\n", false),
        LPartsOfRequest("Host: www.w3.org\n", false),
        LPartsOfRequest("\n", true),
    }, "/query", "GET", "", "HTTP/1.1", {
        LPartsOfRequestQueryParam("somebook", "Hello sss"),
        LPartsOfRequestQueryParam("somebook", "Hello ddd"),
        LPartsOfRequestQueryParam("somebook2", "dmsf"),
        LPartsOfRequestQueryParam("p4", "11_-%dk123")
    }));

    for (int i = 0; i < tests.size(); i++) {
        LTest test = tests[i];
        WsjcppLightWebHttpRequest request(test.sockFd, test.address);
        std::string sNTest = "test" + std::to_string(i) + "#";

        compareS(bTestSuccess, sNTest + " request address", request.getAddress(), test.address);
        compareN(bTestSuccess, sNTest + " request sockfd", request.getSockFd(), test.sockFd);

        for (int n = 0; n < test.parts.size(); n++) {
            std::string sNTest2 = sNTest + " append part" + std::to_string(n) + "#";
            request.appendRecieveRequest(test.parts[n].sPart);
            compareB(bTestSuccess, sNTest2, request.isEnoughAppendReceived(), test.parts[n].enough);
        }
        compareS(bTestSuccess, sNTest + " request expected path", request.getRequestPath(), test.expectedPath);
        compareS(bTestSuccess, sNTest + " request expected type", request.getRequestType(), test.expectedType);
        compareS(bTestSuccess, sNTest + " request expected body", request.getRequestBody(), test.expectedBody);
        compareS(bTestSuccess, sNTest + " request expected body", request.getRequestHttpVersion(), test.expectedHttpVersion);

        std::vector<WsjcppLightWebHttpRequestQueryValue> params = request.getRequestQueryParams();
        for (int i = 0; i < params.size(); i++) {
            WsjcppLightWebHttpRequestQueryValue qv = params[i];
            std::string sExpectedName = "";
            std::string sExpectedValue = "";
            if (i < test.expectedQueryParams.size()) {
                sExpectedName = test.expectedQueryParams[i].sName;
                sExpectedValue = test.expectedQueryParams[i].sValue;
            }
            compareS(bTestSuccess, sNTest + " request query param" + std::to_string(i) + " name", qv.getName(), sExpectedName);
            compareS(bTestSuccess, sNTest + " request query param" + std::to_string(i) + " value", qv.getValue(), sExpectedValue);
        }
        
    }
    return bTestSuccess;
}

