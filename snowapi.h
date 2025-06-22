#ifndef SNOWAPI
#define SNOWAPI

#include "snow.h"

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

typedef struct SnowServer SnowServer;
struct SnowServer {
    int port;
};

void associate_request_handler(char* path, HttpResponse (*handler)(HttpRequest hrequest), int type);
void run_snow_server(SnowServer hserver);

#endif
