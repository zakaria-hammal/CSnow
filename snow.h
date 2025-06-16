#ifndef SNOW
#define SNOW

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

#endif
