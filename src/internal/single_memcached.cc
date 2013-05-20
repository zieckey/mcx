#include "single_memcached.h"
#include "task.h"
#include "memcached_connection.h"

namespace mcx 
{
namespace detail 
{

SingleMemcached::SingleMemcached(const std::string& host, int port)
    : loop_(NULL)
{
    conn_.reset(new MemcachedConnection(host, port));
}

bool SingleMemcached::initialize(EventLoop* loop) {
    loop_ = loop;
    conn_->connect(loop);
    return true;
}

void SingleMemcached::store(
            const std::string& key, 
            const std::string& value,
            const StoreCallback& cb)
{
    LOG_DEBUG << "key=" << key << " value=" << value;
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new StoreTask(conn_->nextSeqNo(), key, value, vbucket_id, cb));
    conn_->run(task);
    //TODO add a timer to handler timeout or other exception
}

void SingleMemcached::remove(const std::string& key,
            const RemoveCallback& cb)
{
    LOG_DEBUG << "key=" << key;
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new RemoveTask(conn_->nextSeqNo(), key, vbucket_id, cb));
    conn_->run(task);
}

void SingleMemcached::get(const std::string& key,
            const GetCallback& cb)
{
    LOG_DEBUG << "key=" << key;
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new GetTask(conn_->nextSeqNo(), key, vbucket_id, cb));
    conn_->run(task);
}

void SingleMemcached::mget(const std::vector<std::string>& keys,
            const MultiGetCallback& cb)
{
}

}//end of namespace detail 

}//end of namespace mcx



