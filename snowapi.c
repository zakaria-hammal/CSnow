#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "snowapi.h"

typedef struct path_handler path_handler;
struct path_handler {
    char* path;
    HttpResponse (*handler)(HttpRequest);
};

typedef struct ListNode ListNode;
struct ListNode {
    path_handler* phandler;
    ListNode* next;
};

typedef struct List List;
struct List {
    ListNode* head;
    int numeber_of_elements;
};

static List post_controllers = {
    .head = NULL,
    .numeber_of_elements = 0
};

static List get_controllers = {
    .head = NULL,
    .numeber_of_elements = 0
};

void associate_request_handler(char* path, HttpResponse (*handler)(HttpRequest hrequest), int type) {
    List controllers;
   
    if (type == POST) {
        controllers = post_controllers;
    }
    else if (type == GET) {
        controllers = get_controllers;
    }
    else {
        perror("Invalide type\n");
        exit(0);
    }

    if (controllers.head == NULL) {
        ListNode *node = malloc(sizeof(ListNode));
        node->next = NULL;
        controllers.numeber_of_elements++;
        controllers.head = node;
        node->phandler = malloc(sizeof(path_handler));
        node->phandler->path = malloc(strlen(path));
        strcpy(node->phandler->path, path);
        node->phandler->handler = handler;
        return;
    }

    ListNode* p = controllers.head;

    while (p) {
        if (!strcmp(p->phandler->path, path)) {
            p->phandler->handler = handler;   
            return;
        }

        p = p->next;
    }
    
    ListNode *node = malloc(sizeof(ListNode));
    node->phandler = malloc(sizeof(path_handler));
    node->phandler->path = malloc(strlen(path));
    strcpy(node->phandler->path, path);
    node->phandler->handler = handler;

    node->next = controllers.head;
    controllers.head = node;
    controllers.numeber_of_elements++;
}

void run_server(HttpServer hserver) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(hserver.port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, SOMAXCONN);

    printf("Server running on port : %d\n", hserver.port);

    int type;
    char* request_buffer;
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        
        char chunk[4096];
        request_buffer = malloc(1);
        request_buffer[0] = '\0';
        int total_received = 0;
        
        while (1) {
            ssize_t n;
            
            n = recv(client_fd, chunk, sizeof(chunk) - 1, 0);
            
            if (n == 0) {
                break;
            }

            chunk[n] = '\0';
            
            char *new_buffer = realloc(request_buffer, total_received + n + 1);
            if (!new_buffer) {
                perror("Realloc failed");
                break;
            }
            request_buffer = new_buffer;
            
            memcpy(request_buffer + total_received, chunk, n);
            total_received += n;
            request_buffer[total_received] = '\0';
            int content_length;

            if (strstr(request_buffer, "\r\n\r\n")) {
                printf("something\n");
                char *content_length_start = strstr(request_buffer, "Content-Length: ");
                if (content_length_start) { 
                    content_length = atoi(content_length_start + 16);
                    printf("%d\n", content_length);
                    printf("%d\n", (int)strlen(strstr(request_buffer, "\r\n\r\n")));
                }

                if (content_length <= strlen(strstr(request_buffer, "\r\n\r\n") + 4)) { 
                    break; 
                }
            }
        }

        printf("%s\n", request_buffer);
        
        if (!strncmp(request_buffer, "GET", strlen("GET"))) {
            type = GET;
        }
        else if (!strncmp(request_buffer, "POST", strlen("POST"))) {
            type = POST;
        }
        else {
            perror("Invalide request");
            free(request_buffer);
            continue;
        }

        char* p;
        size_t len;

        char* path;

        if ((p = strstr(request_buffer, "/"))) {
            len = strcspn(p, &(char){' '});
            path = malloc(len + 1);
            strncpy(path, p, len);
            path[len] = '\0';
        }
        else {
            perror("No path found\n");
            free(request_buffer);
            continue;
        }
        
        char* headers;
        char* body;
        char* header;

        HttpRequest request;
        request.type = type;

        if ((headers = strstr(request_buffer, "\r\n")) && headers != (body = strstr(request_buffer, "\r\n\r\n"))) {
            printf("%p\n", headers);
            headers += 2;

            len = strcspn(headers, "\r\n");
            header = malloc(len + 1);
            strncpy(header, headers, len);
            header[len] = '\0'; 
            
            request.AdditionalHttpRequestHeadersNumber = 1;
            request.AdditionalHttpRequestHeaders = malloc(sizeof(HttpHeader));
            
            len = strcspn(header, ": ");
            strncpy(request.AdditionalHttpRequestHeaders[0].key, header, len);
            header += len + 2;
            len = strcspn(header, "\r\n");
            strncpy(request.AdditionalHttpRequestHeaders[0].value, header, len);

            free(header);

            while((headers = strstr(headers, "\r\n")) && headers != body) {
                headers += 2;
                len = strcspn(headers, "\r\n");
                header = malloc(len + 1);
                strncpy(header, headers, len);
                header[len] = '\0'; 
            
                request.AdditionalHttpRequestHeadersNumber++;
                request.AdditionalHttpRequestHeaders = realloc(request.AdditionalHttpRequestHeaders ,sizeof(HttpHeader) * request.AdditionalHttpRequestHeadersNumber);
            
                len = strcspn(header, ": ");
                strncpy(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].key, header, len);
                header += len + 2;
                len = strcspn(header, "\r\n");
                strncpy(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, header, len);
                
                if (!strcmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].key, "Content-Type")) {
                    if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "application/json", strlen("application/json"))) {
                        request.mime = JSON;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "text/html", strlen("text/html"))) {
                        request.mime = HTML;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "text/css", strlen("text/css"))) {
                        request.mime = CSS;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "application/javascript", strlen("application/javascript")) || !strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "application/js", strlen("application/js"))) {
                        request.mime = JAVASCRIPT;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "text/plain", strlen("text/plain"))) {
                        request.mime = TEXT;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "image/jpeg", strlen("image/jpeg"))) {
                        request.mime = JPEG;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "image/png", strlen("image/png"))) {
                        request.mime = PNG;
                    }
                    else if (!strncmp(request.AdditionalHttpRequestHeaders[request.AdditionalHttpRequestHeadersNumber - 1].value, "application/pdf", strlen("application/pdf"))) {
                        request.mime = PDF;
                    }
                    else {
                        perror("Invalide content type\n");
                        free(header);
                        free(request_buffer);
                        continue;
                    }
                }

                free(header);
            }

            body += 4;
            strncpy(request.HttpRequestBody, body, strlen(body));
        }
        else if (body) {
            body += 4;
            request.HttpRequestBody = malloc(strlen(body) + 1);
            strcpy(request.HttpRequestBody, body);
            request.AdditionalHttpRequestHeaders = NULL;
            request.AdditionalHttpRequestHeadersNumber = 0;

            request.mime = TEXT;
        }
        else {
            perror("Invalide request\n");
            free(request_buffer);
            continue;
        }
        
        ListNode* node;

        if(request.type == POST) {
            node = post_controllers.head;
        }
        else {
            node = get_controllers.head;
        }

        while (node) {
            if (!strcmp(node->phandler->path, path)) {
                break;
            }

            node = node->next;
        }

        if (!node) {
            send(client_fd, "HTTP/1.1 404 Not Found\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\n\r\n"), 0);
            continue;
        }
        else {
            HttpResponse response = node->phandler->handler(request);

            char* status;

            switch (response.StatusCode) {
                // 1xx: Informational
                case 100:
                    status = malloc(strlen("100 Continue") + 1);
                    if (status) strcpy(status, "100 Continue");
                    break;
                case 101:
                    status = malloc(strlen("101 Switching Protocols") + 1);
                    if (status) strcpy(status, "101 Switching Protocols");
                    break;
                case 102:
                    status = malloc(strlen("102 Processing") + 1);
                    if (status) strcpy(status, "102 Processing");
                    break;
                case 103:
                    status = malloc(strlen("103 Early Hints") + 1);
                    if (status) strcpy(status, "103 Early Hints");
                    break;

                // 2xx: Success
                case 200:
                    status = malloc(strlen("200 OK") + 1);
                    if (status) strcpy(status, "200 OK");
                    break;
                case 201:
                    status = malloc(strlen("201 Created") + 1);
                    if (status) strcpy(status, "201 Created");
                    break;
                case 202:
                    status = malloc(strlen("202 Accepted") + 1);
                    if (status) strcpy(status, "202 Accepted");
                    break;
                case 203:
                    status = malloc(strlen("203 Non-Authoritative Information") + 1);
                    if (status) strcpy(status, "203 Non-Authoritative Information");
                    break;
                case 204:
                    status = malloc(strlen("204 No Content") + 1);
                    if (status) strcpy(status, "204 No Content");
                    break;
                case 205:
                    status = malloc(strlen("205 Reset Content") + 1);
                    if (status) strcpy(status, "205 Reset Content");
                    break;
                case 206:
                    status = malloc(strlen("206 Partial Content") + 1);
                    if (status) strcpy(status, "206 Partial Content");
                    break;
                case 207:
                    status = malloc(strlen("207 Multi-Status") + 1);
                    if (status) strcpy(status, "207 Multi-Status");
                    break;
                case 208:
                    status = malloc(strlen("208 Already Reported") + 1);
                    if (status) strcpy(status, "208 Already Reported");
                    break;
                case 226:
                    status = malloc(strlen("226 IM Used") + 1);
                    if (status) strcpy(status, "226 IM Used");
                    break;

                // 3xx: Redirection
                case 300:
                    status = malloc(strlen("300 Multiple Choices") + 1);
                    if (status) strcpy(status, "300 Multiple Choices");
                    break;
                case 301:
                    status = malloc(strlen("301 Moved Permanently") + 1);
                    if (status) strcpy(status, "301 Moved Permanently");
                    break;
                case 302:
                    status = malloc(strlen("302 Found") + 1);
                    if (status) strcpy(status, "302 Found");
                    break;
                case 303:
                    status = malloc(strlen("303 See Other") + 1);
                    if (status) strcpy(status, "303 See Other");
                    break;
                case 304:
                    status = malloc(strlen("304 Not Modified") + 1);
                    if (status) strcpy(status, "304 Not Modified");
                    break;
                case 307:
                    status = malloc(strlen("307 Temporary Redirect") + 1);
                    if (status) strcpy(status, "307 Temporary Redirect");
                    break;
                case 308:
                    status = malloc(strlen("308 Permanent Redirect") + 1);
                    if (status) strcpy(status, "308 Permanent Redirect");
                    break;

                // 4xx: Client Errors
                case 400:
                    status = malloc(strlen("400 Bad Request") + 1);
                    if (status) strcpy(status, "400 Bad Request");
                    break;
                case 401:
                    status = malloc(strlen("401 Unauthorized") + 1);
                    if (status) strcpy(status, "401 Unauthorized");
                    break;
                case 403:
                    status = malloc(strlen("403 Forbidden") + 1);
                    if (status) strcpy(status, "403 Forbidden");
                    break;
                case 404:
                    status = malloc(strlen("404 Not Found") + 1);
                    if (status) strcpy(status, "404 Not Found");
                    break;
                case 405:
                    status = malloc(strlen("405 Method Not Allowed") + 1);
                    if (status) strcpy(status, "405 Method Not Allowed");
                    break;
                case 406:
                    status = malloc(strlen("406 Not Acceptable") + 1);
                    if (status) strcpy(status, "406 Not Acceptable");
                    break;
                case 408:
                    status = malloc(strlen("408 Request Timeout") + 1);
                    if (status) strcpy(status, "408 Request Timeout");
                    break;
                case 409:
                    status = malloc(strlen("409 Conflict") + 1);
                    if (status) strcpy(status, "409 Conflict");
                    break;
                case 410:
                    status = malloc(strlen("410 Gone") + 1);
                    if (status) strcpy(status, "410 Gone");
                    break;
                case 413:
                    status = malloc(strlen("413 Payload Too Large") + 1);
                    if (status) strcpy(status, "413 Payload Too Large");
                    break;
                case 414:
                    status = malloc(strlen("414 URI Too Long") + 1);
                    if (status) strcpy(status, "414 URI Too Long");
                    break;
                case 415:
                    status = malloc(strlen("415 Unsupported Media Type") + 1);
                    if (status) strcpy(status, "415 Unsupported Media Type");
                    break;
                case 429:
                    status = malloc(strlen("429 Too Many Requests") + 1);
                    if (status) strcpy(status, "429 Too Many Requests");
                    break;
                case 451:
                    status = malloc(strlen("451 Unavailable For Legal Reasons") + 1);
                    if (status) strcpy(status, "451 Unavailable For Legal Reasons");
                    break;

                // 5xx: Server Errors
                case 500:
                    status = malloc(strlen("500 Internal Server Error") + 1);
                    if (status) strcpy(status, "500 Internal Server Error");
                    break;
                case 501:
                    status = malloc(strlen("501 Not Implemented") + 1);
                    if (status) strcpy(status, "501 Not Implemented");
                    break;
                case 502:
                    status = malloc(strlen("502 Bad Gateway") + 1);
                    if (status) strcpy(status, "502 Bad Gateway");
                    break;
                case 503:
                    status = malloc(strlen("503 Service Unavailable") + 1);
                    if (status) strcpy(status, "503 Service Unavailable");
                    break;
                case 504:
                    status = malloc(strlen("504 Gateway Timeout") + 1);
                    if (status) strcpy(status, "504 Gateway Timeout");
                    break;
                case 505:
                    status = malloc(strlen("505 HTTP Version Not Supported") + 1);
                    if (status) strcpy(status, "505 HTTP Version Not Supported");
                    break;
                case 507:
                    status = malloc(strlen("507 Insufficient Storage") + 1);
                    if (status) strcpy(status, "507 Insufficient Storage");
                    break;
                case 508:
                    status = malloc(strlen("508 Loop Detected") + 1);
                    if (status) strcpy(status, "508 Loop Detected");
                    break;
                case 511:
                    status = malloc(strlen("511 Network Authentication Required") + 1);
                    if (status) strcpy(status, "511 Network Authentication Required");
                    break;

                default: {
                    perror("Unknown StatusCode");
                    continue;
                    break;
                }
            }

            char mime[50];
        
            switch (response.mime) {
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
                    continue;
                    break;
            }

            char* response_buffer = malloc(strlen(mime) + strlen(status) + strlen(response.HttpResponseMessage) + 512);

            snprintf(response_buffer, sizeof(response_buffer),
                            "HTTP/1.1 %s\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %zu\r\n\r\n"
                            "%s",
                            status, mime, strlen(response.HttpResponseMessage), response.HttpResponseMessage);

            send(client_fd, response_buffer, strlen(response_buffer), 0);

            free(status);
        }

        free(headers);
        free(path);

        printf("%s\n", request_buffer);
        free(request_buffer);
    }

}
