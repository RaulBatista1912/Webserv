# Webserv:
goal: The goal of Webserv is to create your own HTTP server, similar to Nginx or Apache, but in a simplified version.

Your program must be able to:

- Listen on a port (e.g., localhost:8080)
- Accept connections from browsers
- Read an HTTP request
- Understand what the client is requesting
- Send a correct HTTP response

1) Parsing config file:
```
server {
    listen 8080;
    root ./www;
    index index.html;
}
```
It should contain :
- port
- routes
- root
- error pages
- CGI

2) Network Server (sockets):
- socket
- bind
- listen
- accept

and several clients at the same time :
- select
- poll
- or epoll

3) Parsing HTTP Request:

understand the request:
METHOD
PATH
HEADERS
BODY

Example:

element	example:
- Method	GET
- Path	/index.html
- Header	Host
- Body	POST data

4) Response generation:
To Build:
- status line
- headers
- body

5) Advanced Tools:

Selon le sujet :
- CGI
- upload
- delete
- autoindex
- error handlers
- gestion multiple server blocks


# socket:
A socket (literally "plug") is a software interface that allows two programs to communicate with each other over a network. Think of it as the endpoint of a bidirectional communication channel between a client and a server.

For an application to send or receive data, the socket must be uniquely identified on the internet by a combination called the connection interface:

IP ​​Address: Identifies the machine on the network.
Port Number: Identifies the specific application or process on that machine.
Protocol: Usually TCP (connection-oriented, reliable) or UDP (connectionless, fast).

