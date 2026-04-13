# 🌐 Webserv – 42 School Project

## 📌 Overview

Webserv is a **C++ HTTP server** project from 42 School.  
The goal is to build a fully functional **HTTP/1.1 web server from scratch**, similar to NGINX, capable of handling client requests, serving static files, and executing CGI scripts.

This project deepens understanding of:
- HTTP protocol
- Socket programming
- Multiplexing (I/O management)
- Web server architecture

---

## 🎯 Objectives

- Build an HTTP/1.1 compliant server in C++
- Handle multiple clients simultaneously
- Parse configuration files
- Serve static websites
- Support CGI execution
- Manage HTTP methods (GET, POST, DELETE)
- Implement non-blocking I/O

---

## ⚙️ Features

### 🌍 HTTP Server Core
- HTTP/1.1 protocol support
- Request parsing (headers, body, query)
- Response generation with proper status codes
- Keep-alive connection handling

---

### 🔁 Multiplexing
- Uses `poll()` / `select()` / `epoll()` (depending on implementation)
- Handles multiple clients concurrently
- Non-blocking socket communication

---

### 📁 Static File Serving
- Serves HTML, CSS, JS, images
- Configurable root directories
- Directory listing (optional depending on config)

---

### 🧾 HTTP Methods
- `GET` → Retrieve resources
- `POST` → Send data to server
- `DELETE` → Remove resources (if allowed)

---

### ⚙️ CGI Support
- Executes external scripts (Python, PHP, etc.)
- Handles input/output between server and CGI process
- Supports environment variables required by CGI

---

## 🧱 Project Structure

```bash
webserv/
│
├── src/
│   ├── main.cpp
│   ├── Server/
│   ├── Http/
│   ├── Config/
│   ├── Socket/
│   ├── CGI/
│   └── Utils/
│
├── config/
│   └── webserv.conf
│
├── www/
│   ├── index.html
│   ├── styles.css
│   └── assets/
│
├── Makefile
└── README.md
