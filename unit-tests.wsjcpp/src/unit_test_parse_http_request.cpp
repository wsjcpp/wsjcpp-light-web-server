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
    // compareB(bTestSuccess, "Not implemented", true, false);
    return bTestSuccess;
}

