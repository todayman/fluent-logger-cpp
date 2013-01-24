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

    class InterruptedOperation : public ErrnoException {
    public:
        InterruptedOperation() : ErrnoException(EINTR, "[EINTR] Interrupted by signal.") { }
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
        
        class ProtocolType : public ErrnoException {
        public:
            ProtocolType() : ErrnoException(EPROTONOSUPPORT, "[EPROTONOSUPPORT] "
                    "The protocol is not supported within this domain.") { }
        };
        
        class UnsupportedProtocol : public ErrnoException {
        public:
            UnsupportedProtocol() : ErrnoException(EPROTOTYPE, "[EPROTOTYPE] "
                    "The socket type is not supported by the protocol.") { }
        };
        
        class PermissionDenied : public ErrnoException {
        public:
            PermissionDenied() : ErrnoException(EACCES, "[EACCES] "
                    "Permission denied.") { }
        };
        
        class AFNotSupported : public ErrnoException {
        public:
            AFNotSupported() : ErrnoException(EAFNOSUPPORT, "[EAFNOSUPPORT] "
                    "This address family is not supported") { }
        };
        
        class DescriptorTableFull : public ErrnoException {
        public:
            DescriptorTableFull(int err) : ErrnoException(err, "The per-process descriptor table is full") { }
        };
        
        class SystemTableFull : public ErrnoException {
        public:
            SystemTableFull() : ErrnoException(ENFILE, "[ENFILE] "
                    "The system file table is full") { }
        };
        
        class AddressResolutionError : public ::std::runtime_error {
            int ecode;
        public:
            inline AddressResolutionError(int e);
            int getCode() const {
                return ecode;
            }
        };

        class AddressInUse : public ErrnoException {
        public:
            AddressInUse() : ErrnoException(EADDRINUSE, "[EADDRINUSE] "
                    "The address is already in use") { }
        };
        class AddressNotAvailable : public ErrnoException {
        public:
            AddressNotAvailable() : ErrnoException(EADDRNOTAVAIL, "[EADDRNOTAVAIL] "
                    "The specified address is not available.") { }
        };

        class ConnectionRefused : public ErrnoException {
        public:
            ConnectionRefused() : ErrnoException(ECONNREFUSED, "[ECONNREFUSED] "
                    "Connection refused.") { }
        };
        class ConnectionReset : public ErrnoException { 
        public:
            ConnectionReset() : ErrnoException(ECONNRESET, "[ECONNRESET] "
                    "Connection reset.") { }
        };
        class HostUnreachable : public ErrnoException { 
        public:
            HostUnreachable() : ErrnoException(EHOSTUNREACH, "[EHOSTUNREACH] "
                    "Host unreachable") { }
        };
        class NetworkDown : public ErrnoException {
        public:
            NetworkDown() : ErrnoException(ENETDOWN, "[ENETDOWN] "
                    "The network interface is down.") { }
        };
        class NetworkUnreachable : public ErrnoException {
        public:
            NetworkUnreachable() : ErrnoException(ENETUNREACH, "[ENETUNREACH] "
                    "The network is not reachable.") { }
        };
        class ListeningSocket : public ErrnoException {
        public:
            ListeningSocket() : ErrnoException(EOPNOTSUPP, "[EOPNOTSUPP] "
                    "The socket cannot connect because it is listening.") { }
        };
        class TimedOut : public ErrnoException {
        public:
            TimedOut() : ErrnoException(ETIMEDOUT, "[ETIMEDOUT] "
                    "Operation timed out.") { }
        };
        class NoBroadcastOption : public ErrnoException {
        public:
            NoBroadcastOption() : ErrnoException(EACCES, "[EACCES] "
                    "Tried to connect to a broadcast address without the broadcast option set.") { }
        };
        class AlreadyConnecting : public ErrnoException {
        public:
            AlreadyConnecting() : ErrnoException(EALREADY, "[EALREADY] "
                "This socket is nonblocking and already trying to connect") { }
        };
        class WrongAddressType : public ErrnoException {
        public:
            WrongAddressType() : ErrnoException(EPROTOTYPE, "[EPROTOTYPE] "
                    "The target address has a differnt type than the socket") { }
        };
        class NotCompletedYet : public ErrnoException {
        public:
            NotCompletedYet() : ErrnoException(EINPROGRESS, "[EINPROGRESS] "
                    "The socket is non-blocking and the connection cannot be completed immediately.  "
                    "It is possible to select for completion by selecting the socket for writing.") { }
        };
        class InvalidArgs : public ErrnoException {
        public:
            InvalidArgs() : ErrnoException(EINVAL, "[EINVAL] (Internal socket error) "
                    "An invalid argument was passed to a syscall.") { }
        };
        class NoBuffers : public ErrnoException {
        public:
            NoBuffers() : ErrnoException(ENOBUFS, "[ENOBUFS] "
                    "Could not allocate a needed buffer.") { }
        };
        class WouldBlock : public ErrnoException {
        public:
            WouldBlock() : ErrnoException(EAGAIN, "[EAGAIN] "
                    "The socket is non-blocking and this operation would block") { }
        };
        class BadMessageSize : public ErrnoException {
        public:
            BadMessageSize() : ErrnoException(EMSGSIZE, "[EMSGSIZE] "
                    "The socket sends messages atomically, but the message size makes that impossible.") { }
        };
        class BadOptions : public ErrnoException {
        public:
            BadOptions() : ErrnoException(EOPNOTSUPP, "[EOPNOTSUPP] "
                    "The socket does not support all the options specified.") { }
        };
        class NotWritable : public ErrnoException {
        public:
            NotWritable() : ErrnoException(EPIPE, "[EPIPE] "
                    "The socket is shut down for writing or not connected.") { }
        };
        class PreviousWriteError : public ErrnoException {
        public:
            PreviousWriteError() : ErrnoException(EIO, "[EIO] "
                    "A previously uncommitted write encoutered an I/O error") { }
        };
    };
}

#endif /* __SOCKET_H__ */
