#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdexcept>
#include <string>

#include <errno.h>
#include <sys/socket.h>

namespace fluent {
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

        class Exception : public ::std::runtime_error {
        public:
            int err;
            Exception(int e) : ::std::runtime_error("An error occured.  Check this.err"), err(e) { }
        };
    };
}

#endif /* __SOCKET_H__ */
