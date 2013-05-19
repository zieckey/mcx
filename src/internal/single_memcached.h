#ifndef MCX_SINGLE_MEMCACHED_H
#define MCX_SINGLE_MEMCACHED_H

#include "memcached_impl.h"

#include <stdio.h>

#include <boost/bind.hpp>

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

class SingleMemcached : public MemcachedImpl
{
public:
    SingleMemcached(const std::string& host, int port) 
        : loop_(NULL), host_(host), port_(port)
    {}

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
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

private:
    typedef boost::shared_ptr<TcpClient> TcpClientPtr;

private:

    EventLoop*  loop_;
    std::string             host_;
    int                     port_;

    TcpClientPtr            tcp_client_;
};

}//end of namespace detail 

}//end of namespace mcx

#endif // 


