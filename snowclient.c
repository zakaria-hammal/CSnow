#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "snowclient.h"

HttpResponse sendRequest(HttpClient hclient, HttpRequest hrequest) {
    int re_count = 0;
    char type[10];
    char currentUrl[4096];
    
    strcpy(currentUrl, hclient.url);

    HttpResponse response;
    int max = 0;
    
    if(hclient.enable_rediriction) {
        max = hclient.max_redirects;
    }

    while (re_count <= max) {
        switch (hrequest.type) {
            case POST:
                strcpy(type, "POST");
                break;
            case GET:
                strcpy(type, "GET");
                break;
            default:
                perror("Invalide request type\n");
                exit(1);
                break;
        }

        char mime[100];

        switch (hrequest.mime) {
            case JSON:
                strcpy(mime, "application/json");
                break;
            case HTML:
                strcpy(mime, "text/html");
                break;
            case CSS:
                strcpy(mime, "text/css");
                break;
            case JAVASCRIPT:
                strcpy(mime, "application/javascript");
                break;
            case TEXT:
                strcpy(mime, "text/plain");
                break;
            case JPEG:
                strcpy(mime, "image/jpeg");
                break;
            case PNG:
                strcpy(mime, "image/png");
                break;
            case PDF:
                strcpy(mime, "application/pdf");
                break;
            default:
                perror("Invalide mime\n");
                break;
        }
    
        char protocol[10];
        char http[10] = "http://";
        char https[10] = "https://";
        int ishttps = 0;

        if(!strncmp(currentUrl, http, strlen(http))) {
            strcpy(protocol, http);
        }
        else if(!strncmp(currentUrl, https, strlen(https))) {
            strcpy(protocol, https);
            ishttps = 1;
        }
        else {
            perror("Invalide protocol\n");
            exit(0);
        }
    
        char host[4096];
        strcpy(host, strstr(currentUrl, protocol) + strlen(protocol));
        
        char port[10];
        if(strstr(host, ":") && strstr(host, "/")) {
            strncpy(port, strstr(host, ":") + 1, 10);
        }
        else if(ishttps) {
            strcpy(port, "443");
        }
        else {
            strcpy(port, "80");
        }

        char path[4096];
        
        if(strstr(host, "/")) {
            strcpy(path, strstr(host, "/"));
        }
        else {
            strcpy(path, "/");
        }
        
        char* p;
        if((p = strstr(port, "/"))) {
            *p = '\0';
        }

        if((p = strstr(host, ":"))) {
            *p = '\0'; 
        }

        if((p = strstr(host, "/"))) {
            *p = '\0';
        }

        SSL_CTX *ctx = NULL;
        SSL *ssl = NULL;
        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
        setsockopt(client_fd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

        struct sockaddr_in server_address;
        struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
        struct addrinfo *res;
        
        if(strcmp(host, "localhost")){
            if (!ishttps) {
                getaddrinfo(host, port, &hints, &res);
            }
            else {
                getaddrinfo(host, port, &hints, &res);
            }
            connect(client_fd, res->ai_addr, res->ai_addrlen);
            freeaddrinfo(res);
        }
        else {
            server_address.sin_family = AF_INET;
            inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);
            server_address.sin_port = htons(atoi(port));
            connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address));
        }
        
        if (ishttps) {
            OPENSSL_init_ssl(0, NULL);
            ctx = SSL_CTX_new(TLS_client_method());
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client_fd);

            if (SSL_connect(ssl) != 1) {
                ERR_print_errors_fp(stderr);
                exit(EXIT_FAILURE);
            } 
        }
        
        char *request_buffer;
        char additional_headers[4096] = "";
        
        if(hrequest.AdditionalHttpRequestHeaders) {
            for (int i = 0; i < hrequest.AdditionalHttpRequestHeadersNumber; i++) {
                strcat(additional_headers, hrequest.AdditionalHttpRequestHeaders[i].key);
                strcat(additional_headers, ": ");
                strcat(additional_headers, hrequest.AdditionalHttpRequestHeaders[i].value);
                strcat(additional_headers, "\r\n");
            }
        }

        if (hrequest.type == POST) {
            request_buffer = malloc(strlen(hrequest.HttpRequestBody) + 4097);
            
            snprintf(request_buffer, strlen(hrequest.HttpRequestBody) + 4097, "%s %s HTTP/1.1\r\n"
                                "Host: %s\r\n"
                                "UserAgent: snowclient/1.0\r\n"
                                "Accept: */*\r\n"
                                "Connection: close\r\n"
                                "Content-Type: %s\r\n"
                                "%s"
                                "Content-Length: %zu\r\n\r\n"
                                "%s", type, path, host, mime, additional_headers, strlen(hrequest.HttpRequestBody), hrequest.HttpRequestBody);
        }
        else {
            request_buffer = malloc(4097);

            snprintf(request_buffer, 4097,
                                "%s %s HTTP/1.1\r\n"
                                "Host: %s\r\n"
                                "UserAgent: snowclient/1.0\r\n"
                                "Accept: */*\r\n"
                                "Connection: close\r\n"
                                "%s"
                                "Content-Length: 0\r\n\r\n", type, path, host, additional_headers);
        }

        if (ishttps) {
            SSL_write(ssl, request_buffer, strlen(request_buffer));
        }
        else {
            send(client_fd, request_buffer, strlen(request_buffer), 0);
        }
         
        char chunk[4096];
        char *response_buffer;
        response_buffer = malloc(1);
        response_buffer[0] = '\0';
        int total_received = 0;

        while (1) {
            ssize_t n;
            if (ishttps) {
                n = SSL_read(ssl, chunk, sizeof(chunk) - 1);
            }
            else {
                n = recv(client_fd, chunk, sizeof(chunk) - 1, 0);
            }
            
            if (n == 0) {
                break;
            }

            chunk[n] = '\0';
            
            char *new_buffer = realloc(response_buffer, total_received + n + 1);
            if (!new_buffer) {
                perror("Realloc failed");
                break;
            }
            response_buffer = new_buffer;
            
            memcpy(response_buffer + total_received, chunk, n);
            total_received += n;
            response_buffer[total_received] = '\0';
        }
        
        int status = 0;
        char stat[4];

        if (total_received > 12)
        {
            stat[0] = response_buffer[9];
            stat[1] = response_buffer[10];
            stat[2] = response_buffer[11];
            stat[3] = '\0';

            status = atoi(stat);
        }
        
        if(hclient.enable_rediriction && status >= 300 && status < 400) {
            char new_url[4096];

            if (strstr(response_buffer, "Location: ")) {
                strncpy(new_url, strstr(response_buffer, "Location: ") + 10, 4095);
                if (strstr(new_url, "\r\n")) {
                    *strstr(new_url, "\r\n") = '\0';
                }
            
                if (status == 301 || status == 302 || status == 303) {
                    hrequest.type = GET;
                }

                re_count++;
                strcpy(currentUrl, new_url);

                printf("%s\n", currentUrl);

                free(response_buffer);
                continue;
            }
            
        }
        
        char* body = strstr(response_buffer, "\r\n\r\n") + 4;
        
        response.HttpResponseMessage = malloc(strlen(body) * sizeof(char) + 1);
        strcpy(response.HttpResponseMessage, body);
        response.StatusCode = status;
        
        free(response_buffer);
        break;
        close(client_fd);
    }
    
    return response;
}

