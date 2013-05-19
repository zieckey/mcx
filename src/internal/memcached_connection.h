#ifndef MEMCACHED_CONNTION_H_
#define MEMCACHED_CONNTION_H_

#include <string>
#include <map>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>


namespace mcx
{
namespace detail
{

using namespace muduo;
using namespace muduo::net;

class Task;

typedef boost::shared_ptr<Task> TaskPtr;

class MemcachedConnection
{
public:    
    typedef boost::shared_ptr<TcpClient> TcpClientPtr;
    typedef std::map<uint32_t /*task_id*/, TaskPtr> TaskPtrMap;

public:    
    MemcachedConnection(const std::string& host, int port)
        : loop_(NULL), seqno_(0), host_(host), port_(port)
    {}

    bool connect(EventLoop* loop);

    /// get next seqno
    uint32_t nextSeqNo() {
        return seqno_++;
    }

    void run(TaskPtr& task);

    TcpClientPtr& tcp_client() {
        return tcp_client_;
    }
 
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);



private:
    EventLoop*      loop_;
    TcpClientPtr    tcp_client_;
    uint32_t        seqno_; //the sequence number

    std::string     host_;
    int             port_;

    TaskPtrMap      runing_tasks_;
};
typedef boost::shared_ptr<MemcachedConnection> MemcachedConnectionPtr;

}//namespace mcx
}//namespace detail
#endif

