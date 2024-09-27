#ifndef SSLHANDLER_HPP
#define SSLHANDLER_HPP

#include <openssl/ssl.h>
#include <openssl/err.h>

class SSLHandler {
private:
    SSL_CTX* ssl_ctx;
    const char* cert_file;
    const char* key_file;

public:
    SSLHandler(const char* cert_file, const char* key_file);
    ~SSLHandler();

    bool init_ssl_context();
    SSL* create_ssl_connection(int socket_fd);
    
    void cleanup();
};

#endif 