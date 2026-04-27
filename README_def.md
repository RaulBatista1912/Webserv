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
- 200 OK, 404 Not Found, 400 Bad Request

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

# Commands

### GET
GET text: curl -X GET http://localhost:8080/text.txt

GET security: curl --path-as-is http://localhost:8080/../default.conf

Download image: curl http://locahost:8080/images/aaa.jpg -o image.jpg

### POST
POST text: curl -X POST http://localhost:8080/test.txt -d "hello"

POST image: curl -X POST -F "file=@chemin/vers/image.png" http://localhost:8080/uploads/

POST login: curl -c cookies.txt -X POST http://localhost:8080/login -d "user=caca"

POST profile: curl -b cookies.txt http://localhost:8080/profile

### DELETE
curl -X DELETE http://localhost:8080/uploads/img.png

### HEAD
curl --head http://localhost:8080/images/fat.jpg

### Redirections
only headers: curl -v http://localhost:8080/old

headers + body: curl -vL http://localhost:8080/old

### Login
curl -X POST http://localhost:8080/login -d "user=daniel"


#### requêtes rapides
for i in {1..50}; do curl -s http://localhost:8080/session-test & done

Daniel:
- parse empty user
- injections html

