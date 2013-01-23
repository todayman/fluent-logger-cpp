#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdexcept>
#include <string>

#include <errno.h>
#include <sys/socket.h>

namespace fluent {
    class ErrnoException : public ::std::runtime_error {
    private:
        int _err;
    public:
        ErrnoException(int e, const char * str) : ::std::runtime_error(str), _err(e) { }
        int err() const {
            return _err;
        }
    };

    class NoResources : public ErrnoException {
    public:
        NoResources(int err) : ErrnoException(err, "The system is temporarily out of resources.") { }
    };

    class NoMemory : public ErrnoException {
    public:
        NoMemory() : ErrnoException(ENOMEM, "[ENOMEM] There is not enough memory.") { }
    };

    class Socket {
    private:
        int fd;
        bool connected;
    public:
        enum domain_t {
            LOCAL = PF_LOCAL,
            INET = PF_INET,
            ROUTE = PF_ROUTE,
            KEY = PF_KEY,
            INET6 = PF_INET6,
            SYSTEM = PF_SYSTEM,
            NDRV = PF_NDRV,
        };

        enum type_t {
            STREAM = SOCK_STREAM,
            DGRAM = SOCK_DGRAM,
            RAW = SOCK_RAW,
            SEQPACKET = SOCK_SEQPACKET,
            RDM = SOCK_RDM,
        };
        Socket(domain_t domain, type_t type, int protocol = 0);
        ~Socket();

        void settimeout(float timeout);
        void connect(const std::string& host, int port);
        void send(const char * data, size_t length);
        void close();

        operator bool() const {
            return connected;
        }

        class BadFileDescriptor : public ErrnoException {
        private:
            int fd;
        public:
            BadFileDescriptor(int f) : ErrnoException(EBADF, "[EBADF] (Internal socket error) "
                    "The file descriptor used by the socket is no longer valid."), fd(f) { }
            int getFD() const {
                return fd;
            }
        };
        class NotASocket : public ErrnoException {
        private:
            int fd;
        public:
            NotASocket(int f) : ErrnoException(ENOTSOCK, "[ENOTSOCK] (Internal socket error) "
                    "The file descriptor used by the socket is not a socket."), fd(f) { }
            int getFD() const {
                return fd;
            }
        };
        class Exception : public ::std::runtime_error {
        public:
            int err;
            Exception(int e) : ::std::runtime_error("An error occured.  Check this.err"), err(e) { }
        };
        class NotConnected : public ::std::runtime_error {
        public:
            NotConnected() : ::std::runtime_error("The socket is not connected.") { }
        };

        class Connected : public ErrnoException {
        public:
            Connected() : ErrnoException(EISCONN, "[EISCONN] The socket cannot be connected during this operation.") { }
        };

        class InvalidOption : public ErrnoException {
        private:
            int option;
        public:
            InvalidOption(int o) : ErrnoException(EDOM, "[EDOM] (Internal socket error) "
                    "The option queried / set is not valid."), option(o) { }
            int getOption() const {
                return option;
            }
        };
        class InvalidOptionLevel : public ErrnoException {
        public:
            InvalidOptionLevel(int err) : ErrnoException(err, "(Internal socket error) "
                    "The options is not valid at the level indicated.") { }
        };
        class InvalidPointer : public ErrnoException {
        public:
            InvalidPointer() : ErrnoException(EFAULT, "[EFAULT] (Internal socket error) "
                    "Invalid pointers were passed to a syscall.") { }
        };
        
    };
}

#endif /* __SOCKET_H__ */
