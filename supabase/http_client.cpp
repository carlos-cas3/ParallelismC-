#include "http_client.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <regex>

HttpClient::HttpClient(const std::string& url) {
    // Extraer solo el host si viene con https://
    std::regex re("https?://([^/]+)");
    std::smatch match;
    if (std::regex_search(url, match, re)) {
        host = match[1].str();
    } else {
        host = url;
    }
    initSSL();
    std::cout << "[HTTP] Host: " << host << "\n";
}

HttpClient::~HttpClient() {
    if (ctx) SSL_CTX_free(ctx);
}

void HttpClient::initSSL() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
        std::cerr << "[HTTP] SSL_CTX_new failed\n";
}

int HttpClient::connectSocket() {
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        std::cerr << "[HTTP] DNS failed: " << host << "\n";
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(443);
    memcpy(&addr.sin_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[HTTP] TCP connect failed\n";
        close(sock);
        return -1;
    }
    return sock;
}

// Decodifica HTTP chunked transfer encoding
std::string HttpClient::decodeChunked(const std::string& body) {
    std::string result;
    size_t pos = 0;

    while (pos < body.size()) {
        size_t end = body.find("\r\n", pos);
        if (end == std::string::npos) break;

        std::string size_str = body.substr(pos, end - pos);
        size_t chunk_size = std::stoul(size_str, nullptr, 16);

        if (chunk_size == 0) break;

        pos = end + 2;
        if (pos + chunk_size > body.size()) break;

        result += body.substr(pos, chunk_size);
        pos += chunk_size + 2;
    }

    return result;
}

std::string HttpClient::request(
    const std::string& method,
    const std::string& path,
    const std::string& api_key,
    const std::string& body
) {
    int sock = connectSocket();
    if (sock < 0) return "";

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_set_tlsext_host_name(ssl, host.c_str());

    if (SSL_connect(ssl) <= 0) {
        std::cerr << "[HTTP] SSL handshake failed\n";
        SSL_free(ssl);
        close(sock);
        return "";
    }

    std::ostringstream req;
    req << method << " " << path << " HTTP/1.1\r\n"
        << "Host: "               << host    << "\r\n"
        << "apikey: "             << api_key << "\r\n"
        << "Authorization: Bearer " << api_key << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Prefer: return=representation\r\n";
    if (!body.empty())
        req << "Content-Length: " << body.size() << "\r\n";
    req << "Connection: close\r\n\r\n";
    if (!body.empty()) req << body;

    std::string req_str = req.str();
    SSL_write(ssl, req_str.c_str(), req_str.size());

    std::string response;
    char buf[4096];
    int n;
    while ((n = SSL_read(ssl, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        response.append(buf, n);
    }

    SSL_free(ssl);
    close(sock);

    // Separar headers y body
    size_t sep = response.find("\r\n\r\n");
    if (sep == std::string::npos) return response;

    std::string headers  = response.substr(0, sep);
    std::string raw_body = response.substr(sep + 4);

    // Decodificar chunked si es necesario
    if (headers.find("Transfer-Encoding: chunked") != std::string::npos)
        return decodeChunked(raw_body);

    return raw_body;
}
