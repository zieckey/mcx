
#ifndef XASYNC_MEMCACHED_IMPL_H
#define XASYNC_MEMCACHED_IMPL_H

#include "../memcached.h"

#include <stdio.h>

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/InetAddress.h>

namespace mcx 
{

using namespace muduo;
using namespace muduo::net;

class Memcached::Impl 
{
public:
    Impl(const std::string& host, int port) 
        : loop_(NULL), host_(host), port_(port)
    {}

    ~Impl() {}

    bool initialize(EventLoop* loop) {
        loop_ = loop;
        assert(loop_);

        InetAddress addr(host_, static_cast<uint16_t>(port_));
        tcp_client_.reset(new TcpClient(
                        loop, addr, "mcx-memcached-client"));

        tcp_client_->setConnectionCallback(
                    boost::bind(&Impl::onConnection, this, _1));
        tcp_client_->setMessageCallback(
                    boost::bind(&Impl::onMessage, this, _1, _2, _3));

        tcp_client_->connect();

        return true;
    }

    void store(const std::string& key, const std::string& value,
               const StoreCallback& cb)
    {
    }

    void remove(const std::string& key,
                const RemoveCallback& cb)
    {
    }

    void get(const std::string& key,
             const GetCallback& cb)
    {
        char buf[1024] = {};
        snprintf(buf, sizeof(buf), "get %s\r\n", key.data());
        tcp_client_->connection()->send(buf, strlen(buf));
        LOG_TRACE << "request get key=[" << key << "]";
    }

    void mget(const std::vector<std::string>& keys,
              const MultiGetCallback& cb)
    {
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
            conn->setTcpNoDelay(true);
        }
        else
        {
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
    {
        LOG_TRACE << "recv from "
            << conn->peerAddress().toIpPort() 
            << "[" << buf->retrieveAllAsString() << "]";
    }

private:
    void mget(const std::string* keys, size_t key_count, const MultiGetCallback& cb)
    {
    }

private:
    typedef boost::shared_ptr<TcpClient> TcpClientPtr;

private:

    EventLoop*  loop_;
    std::string             host_;
    int                     port_;

    TcpClientPtr            tcp_client_;
};

}

#endif // #ifndef XASYNC_MEMCACHED_H


