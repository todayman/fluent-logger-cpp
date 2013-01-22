#include <cmath>
#include <sstream>
#include <netdb.h>
#include "socket.h"

fluent::Socket::Socket(domain_t domain, type_t type, int protocol)
    : fd(-1), connected(false)
{
    fd = socket(domain, type, protocol);
    if( fd < 0 ) {
        throw Exception(errno);
    }
}

fluent::Socket::~Socket()
{
    close();
}

void fluent::Socket::settimeout(float timeout)
{
    if( fd < 0 ) {
        throw Exception(-1);
    }

    struct timeval s_timeout;
    s_timeout.tv_sec = static_cast<int>(timeout);
    s_timeout.tv_usec = static_cast<int>(::std::fmod(timeout, 1.0f) * 1000000.0);
    int retval = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &s_timeout, sizeof(s_timeout));
    if( retval != 0 ) {
        /* TODO handle errors */
    }

    retval = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &s_timeout, sizeof(s_timeout));
    if( retval != 0 ) {
        /* TODO handle errors */
    }
}

static inline ::std::string itoa(int arg) {
    ::std::ostringstream strm;
    strm << arg;
    return strm.str();
}

void fluent::Socket::connect(const ::std::string& host, int port)
{
    //struct sockaddr_in send_addr;
    struct addrinfo * result = nullptr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = STREAM;

    int retval = getaddrinfo(host.c_str(), itoa(port).c_str(), &hints, &result);

    if( !result ) {
        /* TODO throw some kind of exception */
        return;
    }
    
    /*send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = 0;
    send_addr.sin_port = htons(port);
    retval = ::connect(fd, (struct sockaddr*)&send_addr, sizeof(send_addr));*/
    retval = ::connect(fd, result->ai_addr, result->ai_addrlen);
    
    if( result ) {
        freeaddrinfo(result);
    }

    if( retval < 0 ) {
        throw Exception(errno);
    }
    connected = true;
}

void fluent::Socket::send(const char * data, size_t length)
{
    if( !connected ) {
        /* TODO throw exception */
        return;
    }
    ssize_t retval = ::send(fd, data, length, 0);
    if( retval < 0 ) {
        throw Exception(errno);
        /* TODO handle errors */
    }
}

void fluent::Socket::close()
{
    if( connected ) {
        ::close(fd);
        connected = false;
    }
}
