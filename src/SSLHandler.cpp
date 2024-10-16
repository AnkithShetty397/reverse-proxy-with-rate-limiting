#include "../include/SSLHandler.hpp"
#include <iostream>
#include <cstring>
#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace std;

SSLHandler::SSLHandler(const char* cert_file, const char* key_file):cert_file(cert_file),key_file(key_file),ssl_ctx(NULL) {}

SSLHandler::~SSLHandler(){
    cleanup();
}

bool SSLHandler::init_ssl_context(){
    //initialize OpenSSL libraries
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // create new ssl context
    ssl_ctx = SSL_CTX_new(TLS_server_method());
    if(!ssl_ctx){
        std::cerr<<"Failed to create SSL context."<<std::endl;
        return false;
    }
    
    //load certificate and key file
    if (SSL_CTX_use_certificate_file(ssl_ctx, cert_file, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Failed to load certificate or key file." << std::endl;
        SSL_CTX_free(ssl_ctx);
        return false;
    }

    return true;
}

SSL* SSLHandler::create_ssl_connection(int socket_fd){
    SSL* ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        cerr << "Error creating new SSL" << endl;
        return NULL;
    }

    if (SSL_set_fd(ssl, socket_fd) != 1) {
        cerr << "Error setting file descriptor" << endl;
        SSL_free(ssl);
        return NULL;
    }

    int ret = SSL_accept(ssl);
    if(ret <= 0){
        int err = SSL_get_error(ssl, ret);
        cerr << "Failed to establish SSL connection. Error: " << err << endl;
        ERR_print_errors_fp(stderr);  
        SSL_free(ssl);
        return NULL;
    }

    //cout << "SSL connection established successfully" << endl;
    return ssl;
}

void SSLHandler::cleanup(){
    if(ssl_ctx){
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
    }
    EVP_cleanup();
}