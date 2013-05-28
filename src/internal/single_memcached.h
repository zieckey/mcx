#ifndef MCX_SINGLE_MEMCACHED_H
#define MCX_SINGLE_MEMCACHED_H

#include "memcached_impl.h"

#include <stdio.h>

#include <muduo/base/Logging.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/InetAddress.h>

namespace mcx 
{

namespace detail 
{


using namespace muduo;
using namespace muduo::net;

class MemcachedConnection;
class Task;

class SingleMemcached : public MemcachedImpl
{
public:
    SingleMemcached(Memcached* parent, const std::string& host, int port);

    ~SingleMemcached() {}

    virtual bool initialize(EventLoop* loop);

    virtual void store(const std::string& key, const std::string& value,
               const StoreCallback& cb);

    virtual void remove(const std::string& key,
                const RemoveCallback& cb);

    virtual void get(const std::string& key,
             const GetCallback& cb);
    
    virtual void mget(const std::vector<std::string>& keys,
              const MultiGetCallback& cb);
    
private:
    void runTask(boost::shared_ptr<Task>& task);

private:
    typedef boost::shared_ptr<TcpClient> TcpClientPtr;
    typedef boost::shared_ptr<MemcachedConnection> MemcachedConnectionPtr;

private:
    EventLoop*             loop_;
    MemcachedConnectionPtr conn_;
};

}//end of namespace detail 

}//end of namespace mcx

#endif // 


