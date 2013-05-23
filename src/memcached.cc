
#include "internal/single_memcached.h" 

namespace mcx 
{

Memcached::Memcached(const std::string& host, int port)
    : timeout_ms_(100)
{
    impl_.reset(new detail::SingleMemcached(this, host, port));
}

Memcached::~Memcached()
{
}

bool Memcached::initialize(muduo::net::EventLoop* loop)
{
    return impl_->initialize(loop);
}

void Memcached::store(const std::string& key, const std::string& value,
            const StoreCallback& cb)
{
    impl_->store(key, value, cb);
}

void Memcached::remove(const std::string& key,
            const RemoveCallback& cb)
{
    impl_->remove(key, cb);
}

void Memcached::get(const std::string& key,
            const GetCallback& cb)
{
    impl_->get(key, cb);
}

void Memcached::mget(const std::vector<std::string>& keys,
            const MultiGetCallback& cb)
{
    impl_->mget(keys, cb);
}


}


