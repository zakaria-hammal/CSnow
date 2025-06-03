# CSnow - Lightweight C HTTP Library

CSnow is a lightweight HTTP server and client library written in C, designed for simplicity and efficiency in web development projects.

---

## Features
- **HTTP Server** with GET/POST routing
- **HTTP Client** with SSL/TLS support
- Simple request/response handling
- Support for common MIME types.
- Redirect handling in client
- Header management
- Header management

## Compilation
### Prerequisites
- OpenSSL development libraries
- GCC compiler

### Compilation Instructions
Compile the library and your application together:

```bash
gcc -o myapp myapp.c snowapi.c snowclient.c -lssl -lcrypto
```

## API Reference
### Server API
- > associate_request_handler(path, handler, type)
    Register route handlers for GET/POST requests

- > run_server(server)
    Start HTTP server on specified port

### Client API
- > sendRequest(client, request)
    Send HTTP request and return response
    Handles redirects automatically

### Structures
- > HttpServer: { int port }
- > HttpClient: { char url[], int enable_rediriction, int max_redirects }
- > HttpRequest: { char* body, HttpHeader* headers, int headers_count, int type, int mime }
- > HttpResponse: { char* message, int status, int mime }
