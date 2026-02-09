#pragma once

#include <string>
#include <unistd.h>
#include <sys/socket.h>

class Client {
    public:
        enum State {
            READING,
            WRITING,
            CLOSED
        };
    private:
        int fd;
        State state;
        std::string readBuffer;
        std::string writeBuffer;

    public:
        Client(int fd);
        ~Client();

        bool    readFromSocket();
        bool    writeToSocket();

        void    setState(State s);
        State   getState() const;
        int     getFd() const;

};