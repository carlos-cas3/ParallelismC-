#pragma once
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>

class HttpClient {
public:
    explicit HttpClient(const std::string& host);
    ~HttpClient();

    std::string request(
        const std::string& method,
        const std::string& path,
        const std::string& api_key,
        const std::string& body = ""
    );

private:
    std::string host;
    SSL_CTX* ctx = nullptr;

    int  connectSocket();
    void initSSL();
    std::string decodeChunked(const std::string& body);
};
