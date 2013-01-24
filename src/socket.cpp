#include <cmath>
#include <sstream>
#include <netdb.h>
#include "socket.h"

fluent::Socket::Socket(domain_t domain, type_t type, int protocol)
    : fd(-1), connected(false)
{
    fd = socket(domain, type, protocol);
    if( fd < 0 ) {
        switch(errno) {
            case EACCES:
                throw PermissionDenied();
                break;
            case EAFNOSUPPORT:
                throw AFNotSupported();
                break;
            case EISCONN:
                /* fall through */
            case EMFILE:
                throw DescriptorTableFull(errno);
                break;
            case ENFILE:
                throw SystemTableFull();
                break;
            case ENOBUFS:
                throw NoResources(ENOBUFS);
                break;
            case ENOMEM:
                throw NoMemory();
                break;
            case EPROTONOSUPPORT:
                throw ProtocolType();
                break;
            case EPROTOTYPE:
                throw UnsupportedProtocol();
                break;
            default:
                throw ErrnoException(errno, "Unknown error occured");
        }
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

inline fluent::Socket::AddressResolutionError::AddressResolutionError(int e)
    : ::std::runtime_error(gai_strerror(e)), ecode(e)
{ }

void fluent::Socket::connect(const ::std::string& host, int port)
{
    struct addrinfo * result = nullptr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = STREAM;

    int retval = getaddrinfo(host.c_str(), itoa(port).c_str(), &hints, &result);

    if( retval != 0 ) {
        throw AddressResolutionError(retval);
    }
    
    if( !result ) {
        /* TODO throw some kind of exception */
        throw ::std::runtime_error("(Internal socket error) "
                "getaddrinfo(...) didn't return an error, but the result is NULL");
    }
    
    retval = ::connect(fd, result->ai_addr, result->ai_addrlen);
    
    if( result ) {
        freeaddrinfo(result);
    }

    if( retval < 0 ) {
        /* TODO these are different for UNIX domain sockets */
        switch( errno ) {
            case EACCES:
                throw NoBroadcastOption();
                break;
            case EADDRINUSE:
                throw AddressInUse();
                break;
            case EADDRNOTAVAIL:
                throw AddressNotAvailable();
                break;
            case EAFNOSUPPORT:
                throw AFNotSupported();
                break;
            case EALREADY:
                throw AlreadyConnecting();
                break;
            case EBADF:
                throw BadFileDescriptor(fd);
                break;
            case ECONNREFUSED:
                throw ConnectionRefused();
                break;
            case EFAULT:
                throw InvalidPointer();
                break;
            case EHOSTUNREACH:
                throw HostUnreachable();
                break;
            case EINPROGRESS:
                throw NotCompletedYet();
                break;
            case EINTR:
                throw InterruptedOperation();
                break;
            case EINVAL:
                throw InvalidArgs();
                break;
            case EISCONN:
                throw Connected();
                break;
            case ENETDOWN:
                throw NetworkDown();
                break;
            case ENETUNREACH:
                throw NetworkUnreachable();
                break;
            case ENOBUFS:
                throw NoBuffers();
                break;
            case ENOTSOCK:
                throw NotASocket(fd);
                break;
            case EOPNOTSUPP:
                throw ListeningSocket();
                break;
            case EPROTOTYPE:
                throw WrongAddressType();
                break;
            case ETIMEDOUT:
                throw TimedOut();
                break;
            case ECONNRESET:
                throw ConnectionReset();
                break;
            default:
                throw ErrnoException(errno, "Unknown error occured.  "
                        "This may be because this is a UNIX domain socket "
                        "that returns extra error codes.");
        }
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
        switch( errno ) {
            case EACCES:
                throw NoBroadcastOption();
                break;
            case EAGAIN:
                throw WouldBlock();
                break;
            case EBADF:
                throw BadFileDescriptor(fd);
                break;
            case ECONNRESET:
                throw ConnectionReset();
                break;
            case EFAULT:
                throw InvalidPointer();
                break;
            case EHOSTUNREACH:
                throw HostUnreachable();
                break;
            case EINTR:
                throw InterruptedOperation();
                break;
            case EMSGSIZE:
                throw BadMessageSize();
                break;
            case ENETDOWN:
                throw NetworkDown();
                break;
            case ENETUNREACH:
                throw NetworkUnreachable();
                break;
            case ENOBUFS:
                throw NoBuffers();
                break;
            case ENOTSOCK:
                throw NotASocket(fd);
                break;
            case EOPNOTSUPP:
                throw BadOptions();
                break;
            case EPIPE:
                throw NotWritable();
                break;
            default:
                break;
        }
    }
}

void fluent::Socket::close()
{
    if( connected ) {
        int retval = ::close(fd);
        if( retval < 0 ) {
            switch( errno ) {
                case EBADF:
                    throw BadFileDescriptor(fd);
                    break;
                case EINTR:
                    throw InterruptedOperation();
                    break;
                case EIO:
                    throw PreviousWriteError();
                    break;
                default:
                    throw ErrnoException(errno, "Unknown error occured");
            }
        }
        connected = false;
    }
}
