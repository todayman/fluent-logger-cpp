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

static inline void set_one_timeout(int fd, int which, const struct timeval& s_timeout)
{
    int retval = setsockopt(fd, SOL_SOCKET, which, &s_timeout, sizeof(s_timeout));
    if( retval < 0 ) {
        switch(errno) {
            case EBADF:
                throw ::fluent::Socket::BadFileDescriptor(fd);
                break;
            case EFAULT:
                throw ::fluent::Socket::InvalidPointer();
                break;
            case EINVAL:
                throw ::fluent::Socket::InvalidOptionLevel(EINVAL);
                break;
            case ENOBUFS:
                throw::fluent::NoResources(ENOBUFS);
                break;
            case ENOMEM:
                throw ::fluent::NoMemory();
                break;
            case ENOPROTOOPT:
                throw::fluent::Socket::InvalidOptionLevel(ENOPROTOOPT);
                break;
            case ENOTSOCK:
                throw ::fluent::Socket::NotASocket(fd);
                break;
            case EDOM:
                throw ::fluent::Socket::InvalidOption(which);
                break;
            case EISCONN:
                throw ::fluent::Socket::Connected();
                break;
            default:
                throw ::fluent::ErrnoException(errno, "Unknown error occured");
                break;
        }
    }
}

void fluent::Socket::settimeout(float timeout)
{
    if( fd < 0 ) {
        throw BadFileDescriptor(fd);
    }

    struct timeval s_timeout;
    s_timeout.tv_sec = static_cast<int>(timeout);
    s_timeout.tv_usec = static_cast<int>(::std::fmod(timeout, 1.0f) * 1000000.0);
    set_one_timeout(fd, SO_SNDTIMEO, s_timeout);
    set_one_timeout(fd, SO_RCVTIMEO, s_timeout);
}

static inline ::std::string itoa(int arg) {
    ::std::ostringstream strm;
    strm << arg;
    return strm.str();
}

void fluent::Socket::connect(const ::std::string& host, int port)
{
    struct addrinfo * result = nullptr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = STREAM;

    int retval = getaddrinfo(host.c_str(), itoa(port).c_str(), &hints, &result);

    if( !result ) {
        /* TODO throw some kind of exception */
        return;
    }
    
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
        throw NotConnected();
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
