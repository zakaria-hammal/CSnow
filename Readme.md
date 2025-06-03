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

