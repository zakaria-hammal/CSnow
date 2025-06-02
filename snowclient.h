#ifndef SNOWCLIENT
#define SNOWCLIENT

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

typedef struct HttpClient HttpClient;
struct HttpClient {
    char url[4096];
    int enable_rediriction;
    int max_redirects;
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
    char HttpRequestBody[4096];
    HttpHeader* AdditionalHttpRequestHeaders;
    int AdditionalHttpRequestHeadersNumber;
    int type;
    int mime;
};

HttpResponse sendRequest(HttpClient hclient, HttpRequest hrequest);

#endif
