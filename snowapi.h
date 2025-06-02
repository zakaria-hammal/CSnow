#ifndef SNOWAPI
#define SNOWAPI

#define POST 0
#define GET 1
#define JSON 0
#define HTML 1
#define CSS 2
#define JAVASCRIPT 3
#define TEXT 4
#define JPEG 5
#define PNG 6
#define PDF 7

typedef struct HttpServer HttpServer;
struct HttpServer {
    int port;
};

typedef struct HttpResponse HttpResponse;
struct HttpResponse {
    char* HttpResponseMessage;
    int StatusCode;
    int mime;
};

typedef struct HttpHeader HttpHeader;
struct HttpHeader {
    char key[4096];
    char value[4096];
};

typedef struct HttpRequest HttpRequest;
struct HttpRequest {
    char* HttpRequestBody;
    HttpHeader* AdditionalHttpRequestHeaders;
    int AdditionalHttpRequestHeadersNumber;
    int type;
    int mime;
};

void associate_request_handler(char* path, HttpResponse (*handler)(HttpRequest hrequest), int type);
void run_server(HttpServer hserver);

#endif
