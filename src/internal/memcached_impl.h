#ifndef MCX_MEMCACHED_BASE_IMPL_H
#define MCX_MEMCACHED_BASE_IMPL_H

#include "../memcached.h"

namespace mcx 
{
    
class MemcachedImpl : public boost::noncopyable
{
  public:
    MemcachedImpl(Memcached* m) : parent_(m) {}

    virtual ~MemcachedImpl() {}

    virtual bool initialize(muduo::net::EventLoop* loop) = 0;

    virtual void store(const std::string& key, const std::string& value,
                const StoreCallback& cb) = 0;

    virtual void remove(const std::string& key,
                const RemoveCallback& cb) = 0;

    virtual void get(const std::string& key,
                const GetCallback& cb) = 0;

    virtual void mget(const std::vector<std::string>& keys,
                const MultiGetCallback& cb) = 0;

    //TODO more interface
    
  public: 
    Memcached* parent() const { return parent_; }


  private:
    Memcached* parent_;
};

}

#endif 


