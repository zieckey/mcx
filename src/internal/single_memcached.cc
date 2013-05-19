#include "single_memcached.h"

namespace mcx 
{
namespace detail 
{

bool SingleMemcached::initialize(EventLoop* loop) {
    loop_ = loop;
    assert(loop_);

    InetAddress addr(host_, static_cast<uint16_t>(port_));

    tcp_client_.reset(new TcpClient(
                    loop, addr, "mcx-memcached-client"));

    tcp_client_->setConnectionCallback(
                boost::bind(&SingleMemcached::onConnection, this, _1));
    tcp_client_->setMessageCallback(
                boost::bind(&SingleMemcached::onMessage, this, _1, _2, _3));

    tcp_client_->connect();

    return true;
}

void SingleMemcached::store(const std::string& key, 
            const std::string& value,
            const StoreCallback& cb)
{
}

void SingleMemcached::remove(const std::string& key,
            const RemoveCallback& cb)
{
}

void SingleMemcached::get(const std::string& key,
            const GetCallback& cb)
{
    char buf[1024] = {};
    snprintf(buf, sizeof(buf), "get %s\r\n", key.data());
    tcp_client_->connection()->send(buf, strlen(buf));
    LOG_TRACE << "request get key=[" << key << "]";
}

void SingleMemcached::mget(const std::vector<std::string>& keys,
            const MultiGetCallback& cb)
{
}

void SingleMemcached::onConnection(const TcpConnectionPtr& conn)
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

void SingleMemcached::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    LOG_TRACE << "recv from "
        << conn->peerAddress().toIpPort() 
        << "[" << buf->retrieveAllAsString() << "]";
}

}//end of namespace detail 

}//end of namespace mcx



