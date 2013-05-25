#include "single_memcached.h"

#include <boost/bind.hpp>

#include "task.h"
#include "memcached_connection.h"


namespace mcx 
{
namespace detail 
{

SingleMemcached::SingleMemcached(Memcached* p, const std::string& host, int port)
    : MemcachedImpl(p), loop_(NULL)
{
    conn_.reset(new MemcachedConnection(host, port));
}

bool SingleMemcached::initialize(EventLoop* loop) {
    loop_ = loop;
    conn_->connect(loop);
    return true;
}

struct TimeoutCallback {
    TimeoutCallback(MemcachedConnection* conn, TaskPtr& task) : conn_(conn), task_(task) {}

    void operator()() const {
        task_->onTimeout();
        conn_->cancelTask(task_);
    }

  private:
    MemcachedConnection* conn_;
    TaskPtr task_;
};

void SingleMemcached::runTask(TaskPtr& task)
{
    TimerId tid = loop_->runAfter(double(parent()->getTimeout())/1000, TimeoutCallback(conn_.get(), task)); 
    task->setTimerId(tid);
    conn_->run(task);
}

void SingleMemcached::store(
            const std::string& key, 
            const std::string& value,
            const StoreCallback& cb)
{
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new StoreTask(conn_->nextSeqNo(), key, value, vbucket_id, cb));
    loop_->runInLoop(boost::bind(&SingleMemcached::runTask, this, task));
}

void SingleMemcached::remove(const std::string& key,
            const RemoveCallback& cb)
{
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new RemoveTask(conn_->nextSeqNo(), key, vbucket_id, cb));
    loop_->runInLoop(boost::bind(&SingleMemcached::runTask, this, task));
}

void SingleMemcached::get(const std::string& key,
            const GetCallback& cb)
{
    uint16_t vbucket_id = 0;//TODO calculate vbucket id
    TaskPtr task(new GetTask(conn_->nextSeqNo(), key, vbucket_id, cb));
    loop_->runInLoop(boost::bind(&SingleMemcached::runTask, this, task));
}

void SingleMemcached::mget(const std::vector<std::string>& keys,
            const MultiGetCallback& cb)
{
    std::vector<std::string>::const_iterator it (keys.begin());
    std::vector<std::string>::const_iterator ite(keys.end());
    std::vector<KeyEntry> key_entrys;
    for (; it != ite; ++it) {
        uint16_t vbucket_id = 0;//TODO calculate vbucket id
        KeyEntry e(*it, vbucket_id);
        key_entrys.push_back(e);
    }
    TaskPtr task(new MultiGetTask(conn_->nextSeqNo(), key_entrys, cb));
    loop_->runInLoop(boost::bind(&SingleMemcached::runTask, this, task));
}


}//end of namespace detail 

}//end of namespace mcx



