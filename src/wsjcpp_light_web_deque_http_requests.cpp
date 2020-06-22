#include "wsjcpp_light_web_deque_http_requests.h"

#include <wsjcpp_core.h>

// ----------------------------------------------------------------------
// WsjcppLightWebDequeHttpRequests

WsjcppLightWebDequeHttpRequests::WsjcppLightWebDequeHttpRequests() {
    TAG = "WsjcppLightWebDequeHttpRequests";
    m_bLoggerEnabled = false;
}

// ----------------------------------------------------------------------

WsjcppLightWebHttpRequest *WsjcppLightWebDequeHttpRequests::popRequest() {
    if (m_dequeRequests.size() == 0) {
        m_mtxWaiterRequests.try_lock();
    }
    std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
    WsjcppLightWebHttpRequest *pRequest = nullptr;
    int nSize = m_dequeRequests.size();
    if (nSize > 0) {
        pRequest = m_dequeRequests.back();
        m_dequeRequests.pop_back();
    }
    return pRequest;
}

// ----------------------------------------------------------------------

void WsjcppLightWebDequeHttpRequests::pushRequest(WsjcppLightWebHttpRequest *pRequest) {
    
    std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
    int nPreviousSize = m_dequeRequests.size();
    if (nPreviousSize > 20 && m_bLoggerEnabled) {
        WsjcppLog::warn(TAG, " deque more than " + std::to_string(nPreviousSize));
    }
    m_dequeRequests.push_front(pRequest);
    if (nPreviousSize == 0) {
        m_mtxWaiterRequests.unlock();
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebDequeHttpRequests::cleanup() {
    std::lock_guard<std::mutex> guard(this->m_mtxDequeRequests);
    while (m_dequeRequests.size() > 0) {
        delete m_dequeRequests.back();
        m_dequeRequests.pop_back();
    }
}

// ----------------------------------------------------------------------

void WsjcppLightWebDequeHttpRequests::setLoggerEnable(bool bEnable) {
    m_bLoggerEnabled = bEnable;
}

// ----------------------------------------------------------------------

void WsjcppLightWebDequeHttpRequests::addKeepAliveSocket(int m_nSockFd) {
    WsjcppLog::warn(TAG, "WsjcppLightWebDequeHttpRequests::addKeepAliveSocket not implemented");
}