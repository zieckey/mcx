#include "memcached_connection.h"

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/InetAddress.h>
#include "task.h"

namespace mcx 
{
namespace detail 
{

bool MemcachedConnection::connect(EventLoop* loop) {
    loop_ = loop;

    assert(loop_);

    InetAddress addr(host_, static_cast<uint16_t>(port_));

    tcp_client_.reset(new TcpClient(loop, addr, "mcx-memcached-client"));

    tcp_client_->setConnectionCallback(
                boost::bind(&MemcachedConnection::onConnection, this, _1));
    tcp_client_->setMessageCallback(
                boost::bind(&MemcachedConnection::onMessage, this, _1, _2, _3));

    tcp_client_->connect();

    return true;
}

void MemcachedConnection::run(TaskPtr& task)
{
    runing_tasks_[task->id()] = task;
    task->run(this);
}

//void MemcachedConnection::store(const std::string& key, 
//            const std::string& value,
//            const StoreCallback& cb)
//{
//}
//
//void MemcachedConnection::remove(const std::string& key,
//            const RemoveCallback& cb)
//{
//}
//
//void MemcachedConnection::get(const std::string& key,
//            const GetCallback& cb)
//{
//    char buf[1024] = {};
//    snprintf(buf, sizeof(buf), "get %s\r\n", key.data());
//    tcp_client_->connection()->send(buf, strlen(buf));
//    LOG_TRACE << "request get key=[" << key << "]";
//}
//
//void MemcachedConnection::mget(const std::vector<std::string>& keys,
//            const MultiGetCallback& cb)
//{
//}
//
void MemcachedConnection::onConnection(const TcpConnectionPtr& conn)
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

void MemcachedConnection::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    LOG_DEBUG << "recv " << buf->readableBytes() << " bytes from " 
        << conn->peerAddress().toIpPort();// << "[" << buf->retrieveAllAsString() << "]";
}

}//end of namespace detail 

}//end of namespace mcx



