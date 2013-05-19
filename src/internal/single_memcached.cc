#include "single_memcached.h"
#include "task.h"
#include "memcached_connection.h"

namespace mcx 
{
namespace detail 
{

SingleMemcached::SingleMemcached(const std::string& host, int port)
{
    conn_.reset(new MemcachedConnection(host, port));
}

bool SingleMemcached::initialize(EventLoop* loop) {
    conn_->connect(loop);
    return true;
}

void SingleMemcached::store(
            const std::string& key, 
            const std::string& value,
            const StoreCallback& cb)
{
    LOG_DEBUG << "store key=" << key << " value=" << value;
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new StoreTask(conn_->nextSeqNo(), key, value, vbucket_id, cb));
    conn_->run(task);
    //TODO add a timer to handler timeout or other exception
}

void SingleMemcached::remove(const std::string& key,
            const RemoveCallback& cb)
{
}

void SingleMemcached::get(const std::string& key,
            const GetCallback& cb)
{
    //char buf[1024] = {};
    //snprintf(buf, sizeof(buf), "get %s\r\n", key.data());
    //tcp_client_->connection()->send(buf, strlen(buf));
    //LOG_TRACE << "request get key=[" << key << "]";
}

void SingleMemcached::mget(const std::vector<std::string>& keys,
            const MultiGetCallback& cb)
{
}

}//end of namespace detail 

}//end of namespace mcx



