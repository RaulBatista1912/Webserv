*This project has been created as part of the 42 curriculum by <login1>, <login2>.*

# 🌐 Webserv

## 📖 Description

**Webserv** is a custom HTTP/1.1 web server written in C++ (C++98 standard).  
The goal of this project is to understand how web servers work internally by recreating core features of servers like Nginx or Apache.

The server is capable of handling multiple clients simultaneously using non-blocking I/O (`poll()`), parsing HTTP requests, and generating appropriate responses.

### Core Features

- HTTP/1.1 request parsing (headers + body)
- Non-blocking I/O using `poll()`
- Static file serving (GET)
- File upload support (POST with multipart/form-data)
- DELETE method support
- Custom error pages
- Autoindex (directory listing)
- Redirections (301)
- Multiple server blocks (multi-host / multi-port)
- Config file parsing (Nginx-like)

### 🚀 Bonus Features

- Multiple CGI support (e.g. PHP, Python scripts)
- Session management with cookies
- Login / logout system
- Basic authentication logic
- Secure cookie handling (`HttpOnly`, `SameSite`)
- Input validation and sanitization

---

## ⚙️ Instructions

### 🛠️ Compilation

```bash
make
./webserv default.conf
```

### Example config
```bash
server {
    listen 8080;
    root www;

    error_page 404 /errors/404.html;

    location / {
        index index.html;
    }

    location /upload {
        allowed_methods POST;
    }

    location /cgi-bin {
        cgi_extension .py;
        cgi_path /usr/bin/python3;
    }
}
```
### 🧪 Usage Examples

#### GET
```bash
curl -X GET http://localhost:8080
```
#### POST
```bash
curl -d "user=daniel" http://localhost:8080/login
```
#### FILE UPLOAD
```bash
curl -F "file=@test.txt" http://localhost:8080/uploads
```
#### DELETE
```bash
curl -X DELETE http://localhost:8080/uploads/img.jpg
```
#### Session handling
```bash
curl -c cookies.txt -d "user=daniel" http://localhost:8080/login
curl -b cookies.txt http://localhost:8080/profile
```

### 🧠 Technical Choices
- ++98: imposed constraint for low-level control
- poll(): efficient handling of multiple clients without threads
- Modular architecture:
- - Client handles connection lifecycle
- - Request parses HTTP
- - Response builds HTTP responses
- - SessionManager handles sessions
- Security considerations:
- - Path traversal protection (../)
- - Input validation (username sanitization)
- - Cookie flags (HttpOnly, SameSite)
- Stateless HTTP + sessions:
- - Session IDs stored in cookies
- - Server-side session storage

## 📚 Resources

### 📖 Documentation

- RFC 7230–7235 (HTTP/1.1)
- https://developer.mozilla.org (MDN Web Docs)
- https://nginx.org/en/docs/
- https://man7.org/linux/man-pages/man2/poll.2.html

---

### 🎓 Tutorials & Articles

- “How HTTP Works” – MDN
- Beej’s Guide to Network Programming
- Nginx architecture explanations

---

### 🤖 AI Usage

AI tools (such as ChatGPT) were used to:

- Understand HTTP protocol behavior and edge cases
- Design session and cookie handling
- Debug request parsing and multipart handling
- Improve code structure and architecture
- Generate UI examples (HTML/Tailwind)
- Clarify networking concepts (`poll`, sockets, etc.)

All generated suggestions were reviewed, adapted, and implemented manually.

---

## 📌 Notes

This project focuses on **understanding fundamentals**, not performance or production readiness.  
It demonstrates how a real web server works internally, including request parsing, routing, and session management.

---

## 👥 Authors

- Rjot90
- RaulBatista1912
- daniel149afonso