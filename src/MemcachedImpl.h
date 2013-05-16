
#ifndef XASYNC_MEMCACHED_IMPL_H
#define XASYNC_MEMCACHED_IMPL_H

namespace mcx 
{

class MemcachedImpl : public boost::noncopyable
{
public:
    MemcachedImpl(const std::string& host, int port) {
        : loop_(NULL), host_(host), port_(port)
    }

    ~MemcachedImpl() {}

    bool initialize(muduo::net::EventLoop* loop) {
        loop_ = loop;
        assert(loop_);
        //TODO set callback
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
    }

    void mget(const std::vector<std::string>& keys,
              const MultiGetCallback& cb)
    {
    }

private:


private:
    muduo::net::EventLoop*  loop_;
    std::string             host_;
    int                     port_;
};

}

#endif // #ifndef XASYNC_MEMCACHED_H


