#include "memcached_connection.h"

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/InetAddress.h>
#include "task.h"
#include "binary_codec.h"

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

    codec_.reset(new BinaryCodec(this));
    tcp_client_->setMessageCallback(
                boost::bind(&BinaryCodec::onMessage, codec_.get(), _1, _2, _3));

    tcp_client_->connect();

    return true;
}

void MemcachedConnection::run(TaskPtr& task)
{
    running_tasks_[task->id()] = task;
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

void MemcachedConnection::onStoreTaskDone(int task_id, const Status& status) 
{
    TaskPtrMap::iterator it = running_tasks_.find(task_id);
    assert(it != running_tasks_.end());
    if (it == running_tasks_.end()) 
    {
        LOG_ERROR << "StoreTask task_id=" << task_id << " NOT FOUND";
        return;
    }

    assert(dynamic_cast<StoreTask*>(it->second.get()));
    StoreTask* task = static_cast<StoreTask*>(it->second.get());
    task->report(status);
}

}//end of namespace detail 

}//end of namespace mcx



