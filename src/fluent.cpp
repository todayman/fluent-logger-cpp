#include <errno.h>

#include <iomanip>

#include "fluent_cpp.h"

fluent::Sender::Sender(
                const std::string& h, int p,
                size_t b, float _timeout, bool v)
    :  host(h), port(p), bufmax(b), timeout(_timeout), verbose(v),
        buf(nullptr), sock(Socket::INET, Socket::STREAM)
{
#ifdef FLUENT_MT
    int retval = pthread_mutex_init(&mutex, NULL);
    switch(retval)
    {
        case 0:
            /* success! */
            break;
        case EAGAIN:
            throw NoResources();
            break;
        case EINVAL:
            throw InvalidAttributes();
            break;
        case ENOMEM:
            throw NoMemory();
            break;
        default:
            throw UnknownError(retval);
    }
#endif
    try {
        reconnect();
    }
    catch(...) {
        close();
    }
}

fluent::Sender::~Sender()
{
#ifdef FLUENT_MT
    int retval = pthread_mutex_destroy(&mutex);
    switch(retval) {
        case 0:
            /* success! */
            break;
        case EBUSY:
            /* mutex was locked, but we're being destroyed,
             * this is really weird.
             * I guess we're being destroyed while someone else is
             * trying to log things.  They screwed up already.
             * Or, we didn't release the lock when we should have.
             * So, log to stderr, since we're the logger...
             * TODO what happens if I throw an exception here?
             * Exploding everything / core dump would be excellent */
            std::cerr << "The fluent::Sender (0x" << std::hex << this << std::dec << ") is being destroyed, but someone is holding my lock.  This is really bad.  You (or the person who wrote fluent::Sender) should fix this.  But probably you.\n";
            break;
        case EINVAL:
            std::cerr << "The fluent::Sender (0x" << std::hex << this << std::dec << ") is being destroyed, but it's mutex is invalid.  Someone corrupted my mutex.  This is really bad.  You (or the person who wrote fluent::Sender) should fix this.  But probably you.\n";
            break;
        default:
            std::cerr << "The fluent::Sender (0x" << std::hex << this << std::dec << ") is being destroyed and got an unrecognized error (" << retval << ") attempting to destroy its mutex.\n";
    }
#endif
}

void fluent::Sender::send(const ::msgpack::sbuffer& bytes)
{
#ifdef FLUENT_MT
    /* TODO make this exception safe */
    pthread_mutex_lock(&mutex);
#endif
    send_internal(bytes);
#ifdef FLUENT_MT
    pthread_mutex_unlock(&mutex);
#endif
}

void fluent::Sender::send_internal(const ::msgpack::sbuffer& bytes)
{
    const ::msgpack::sbuffer * to_send = nullptr;
    if( !buf )
    {
        to_send = &bytes;
    }
    else {
        buf->write(bytes.data(), bytes.size());
        to_send = buf;
    }
    try {
        reconnect();
        sock.send(to_send->data(), to_send->size());
        /* TODO should I use release or clear here?
         * release frees the memory in the sbuffer,
         * clear just empties out the buffer. */
        if( buf ) {
            delete buf;
        }
    }
    catch(::std::runtime_error& e) {
        ::std::cerr << "while sending, got exception " << e.what() << "\n";
        close();
        if( buf ) {
            if(  buf->size() > bufmax ) {
                /* buffer is already full, so drop everything */
                /* python says put a callback here */
                delete buf;
            }
        }
        else {
            buf = new ::msgpack::sbuffer();
            buf->write(bytes.data(), bytes.size());
        }
        throw;
    }
}

void fluent::Sender::reconnect()
{
    if( !sock ) {
        sock.settimeout(timeout);
        sock.connect(host, port);
    }
}

void fluent::Sender::close()
{
    if( sock ) {
        sock.close();
    }
}

