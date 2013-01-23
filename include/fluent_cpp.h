#ifndef __FLUENT_CPP_H__
#define __FLUENT_CPP_H__

#include <time.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <msgpack.hpp>

#ifdef FLUENT_MT
#include <pthread.h>
#endif

#include "socket.h"

namespace fluent {
    
    class Sender {
    protected:
        std::string host;
        int port;
        size_t bufmax;
        float timeout;
        bool verbose;

        msgpack::sbuffer * buf;
        Socket sock;
#ifdef FLUENT_MT
        // TODO RAII on the mutex and the locks
        pthread_mutex_t mutex;
#endif
        
    public:
        Sender(const std::string& h = std::string("localhost"), int p = 24224,
                size_t b = 1024*1024, float _timeout = 3.0, bool v = false);
        ~Sender();

        bool emit(const ::msgpack::sbuffer& sbuf)
        {
            send(sbuf);
            return true;
            /* TODO this might throw exceptions when I write socket / connect / etc. */
        }

    protected:
        void send(const ::msgpack::sbuffer& sbuf);
        void send_internal(const ::msgpack::sbuffer& sbuf);
        void reconnect();
        void close();
    };

    class Logger {
    private:
        std::string prefix;
        Sender sender;
        
    public:
        /* TODO figure out tag / prefix nonsense */
        Logger(const std::string& t,
                const std::string& h = std::string("localhost"), int p = 24224,
                size_t b = 1024*1024, float _timeout = 3.0, bool v = false)
        : prefix(t), sender(h, p, b, _timeout, v) { }
        
    private:
        void add_args(msgpack::packer<msgpack::sbuffer>& packer, const std::string& key, const char* value)
        {
            packer.pack(key);
            packer.pack(::std::string(value));
        }
        
        template<typename V>
        void add_args(msgpack::packer<msgpack::sbuffer>& packer, const std::string& key, const V& value)
        {
            packer.pack(key);
            packer.pack(value);
        }
        
        template<typename V, typename... Params>
        void add_args(msgpack::packer<msgpack::sbuffer>& packer, const std::string& key, const V& value, Params... parameters)
        {
            packer.pack(key);
            packer.pack(value);
            add_args(packer, parameters...);
        }
        
        template<typename... Params>
        void add_args(msgpack::packer<msgpack::sbuffer>& packer, const std::string& key, const char * value, Params... parameters)
        {
            packer.pack(key);
            packer.pack(::std::string(value));
            add_args(packer, parameters...);
        }
        
    public:
        template<typename... Params>
        bool log(const std::string& label, Params... parameters)
        {
            return log(label, ::time(NULL), parameters...);
        }
        
        template<typename... Params>
        bool log(const std::string& label, time_t timestamp, Params... parameters)
        {
            msgpack::sbuffer sbuf;
            msgpack::packer<msgpack::sbuffer> packer(sbuf);
            packer.pack_array(3);
            if( prefix.size() ) {
                if( label.size() ) {
                    packer.pack(prefix + "." + label);
                }
                else {
                    packer.pack(prefix);
                }
            }
            else {
                if( label.size() ) {
                    packer.pack(label);
                }
                else {
                    /* TODO is this an error */
                    packer.pack(::std::string());
                }
            }
            packer.pack(timestamp);
            packer.pack_map(sizeof...(Params) / 2);
            add_args(packer, parameters...);
            return sender.emit(sbuf);
        }
        
    };

#ifdef FLUENT_MT
    /* Errors using the mutex */
    class InvalidAttributes : public std::runtime_error {
    public:
        InvalidAttributes() : std::runtime_error("Cannot create a mutex with the (NULL) attribute.") { }
    };
    class UnknownError : public std::runtime_error {
    public:
        int err;
        UnknownError(int e) : std::runtime_error("Unknown error creating the mutex."), err(e) { }
    };
#endif
    
}

#endif /* __FLUENT_CPP_H__ */
