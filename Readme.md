# CSnow v1.0 ❄️

**CSnow** is a lightweight web development library written in C, designed to offer a low-level interface for handling basic client-server communication. It's ideal for learning network programming or building minimal C-based web services.

## Features

- Simple and clean C interface
- Lightweight and dependency-free
- Server and client-side modules
- Easy to integrate into any C project

## Getting Started

To use CSnow in your project, clone or download the repository and compile the source files along with your code.

### Requirements

- GCC (or any C99-compliant compiler)
- POSIX-compatible system (for socket operations)

---

## 🔧 Compilation Instructions

To compile the **server**:

```bash
gcc -o myapp myapp.c snowapi.c snowclient.c -lssl -lcrypto
