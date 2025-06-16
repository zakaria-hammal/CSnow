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

typedef struct SnowClient SnowClient;
struct SnowClient {
    char url[4096];
    int enable_rediriction;
    int max_redirects;
};

HttpResponse send_request(SnowClient hclient, HttpRequest hrequest);

#endif
