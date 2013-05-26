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

MemcachedConnection::MemcachedConnection(const std::string& srv_host, int listen_port)
    : loop_(NULL), host_(srv_host), port_(listen_port)
{}

MemcachedConnection::~MemcachedConnection()
{
    assert(running_tasks_.empty());
    assert(mget_running_tasks_.empty());
}

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

void MemcachedConnection::rotateSeqNo()
{
    //reserve 65536 ids
    static const int32_t kMaxSeqNo = 0x7fff0000;
    if (seqno_.get() > kMaxSeqNo) {
        seqno_.getAndSet(0);
    }
}

void MemcachedConnection::run(TaskPtr& task)
{
    task->run(this);
    if (task->isMultiGet()) {
        //insert task->id, and all the get-cmd-id, and also the last noop-cmd-id
        mget_running_tasks_[task->id()] = task;
            LOG_TRACE << "add multi-get id=" << task->id();
        MultiGetTask* mtask = static_cast<MultiGetTask*>(task.get());
        for (uint32_t id = mtask->getFristGetTaskId(); id <= mtask->getNoopTaskId(); ++id) {
            mget_running_tasks_[id] = task;
        }
    } else { 
        running_tasks_[task->id()] = task;
    }

    rotateSeqNo();
}

void MemcachedConnection::cancelTask(const TaskPtr& task)
{
    removeTask(task);
    LOG_TRACE << "cancelTask id=" << task->id();
}

void MemcachedConnection::removeTask(const TaskPtr& task)
{
    if (task->isMultiGet()) {
        mget_running_tasks_.erase(task->id());
            LOG_TRACE << "remove multi-get id=" << task->id();
        MultiGetTask* mtask = static_cast<MultiGetTask*>(task.get());
        for (uint32_t id = mtask->getFristGetTaskId(); id <= mtask->getNoopTaskId(); ++id) {
            mget_running_tasks_.erase(id);
            LOG_TRACE << "remove multi-get id=" << id;
        }
    } else { 
        running_tasks_.erase(task->id());
    }
}

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
        //TODO
    }
}

void MemcachedConnection::onStoreTaskDone(uint32_t task_id, int memcached_response_code) 
{
    onTaskDone<StoreTask>(task_id, memcached_response_code);
}

void MemcachedConnection::onRemoveTaskDone(uint32_t task_id, int memcached_response_code) 
{
    onTaskDone<RemoveTask>(task_id, memcached_response_code);
}

template< class TaskT >
void MemcachedConnection::onTaskDone(uint32_t task_id, int memcached_response_code)
{
    TaskPtrMap::iterator it = running_tasks_.find(task_id);
    if (it == running_tasks_.end()) 
    {
        LOG_ERROR << "task_id=" << task_id << " NOT FOUND, maybe timeout!";
        return;
    }

    loop_->cancel(it->second->getTimerId());

    assert(dynamic_cast<TaskT*>(it->second.get()));
    TaskT* task = static_cast<TaskT*>(it->second.get());
    assert(task_id == task->id());
    switch (memcached_response_code) {
        case PROTOCOL_BINARY_RESPONSE_SUCCESS:
            task->report(Status(Status::kOK, memcached_response_code));
            break;
        case PROTOCOL_BINARY_RESPONSE_KEY_ENOENT:
            task->report(Status(Status::kNotFound, memcached_response_code));
            break;
        default:
            task->report(Status(Status::kNetworkError, memcached_response_code));
            break;

    }

    running_tasks_.erase(it);
}

void MemcachedConnection::onGetTaskDone(uint32_t task_id, 
            int memcached_response_code, const std::string& return_value)
{
    TaskPtrMap::iterator it = running_tasks_.find(task_id);
    if (it == running_tasks_.end()) 
    {
        //LOG_ERROR << "task_id=" << task_id << " NOT FOUND, maybe timeout!";
        return;
    }

    loop_->cancel(it->second->getTimerId());

    assert(dynamic_cast<GetTask*>(it->second.get()));
    GetTask* task = static_cast<GetTask*>(it->second.get());
    assert(task_id == task->id());
    Status status(Status::kOK, memcached_response_code);
    switch (memcached_response_code) {
        case PROTOCOL_BINARY_RESPONSE_SUCCESS:
            task->report(GetResult(status, return_value));
            break;
        case PROTOCOL_BINARY_RESPONSE_KEY_ENOENT:
            status.setCode(Status::kNotFound);
            task->report(GetResult(status, ""));
            break;
        default:
            status.setCode(Status::kNetworkError);
            task->report(GetResult(status, ""));
            break;

    }

    running_tasks_.erase(it);
}

void MemcachedConnection::onMultiGetTaskOneResponse(uint32_t cmd_id, 
            int memcached_response_code, const std::string& return_value)
{
    TaskPtrMap::iterator it = mget_running_tasks_.find(cmd_id);
    if (it == mget_running_tasks_.end()) 
    {
        LOG_ERROR << "cmd_id=" << cmd_id << " NOT FOUND, maybe timeout!";
        return;
    }

    MultiGetTask* task = static_cast<MultiGetTask*>(it->second.get());
    
    switch (memcached_response_code) {
        case PROTOCOL_BINARY_RESPONSE_SUCCESS:
            task->onResult(cmd_id, return_value, Status(Status::kOK, memcached_response_code));
            break;
        case PROTOCOL_BINARY_RESPONSE_KEY_ENOENT:
            task->onResult(cmd_id, return_value, Status(Status::kNotFound, memcached_response_code));
            break;
        default:
            task->onResult(cmd_id, return_value, Status(Status::kNetworkError, memcached_response_code));
            break;
    }

    mget_running_tasks_.erase(it);
}

void MemcachedConnection::onMultiGetTaskDone(uint32_t noop_cmd_id, int memcached_response_code)
{
    TaskPtrMap::iterator it = mget_running_tasks_.find(noop_cmd_id);
    if (it == mget_running_tasks_.end()) 
    {
        LOG_ERROR << "cmd_id=" << noop_cmd_id << " NOT FOUND, maybe timeout!";
        return;
    }

    loop_->cancel(it->second->getTimerId());
    loop_->cancel(it->second->getTimerId());

    MultiGetTask* task = static_cast<MultiGetTask*>(it->second.get());
    assert(task->getNoopTaskId() == noop_cmd_id);

    for (uint32_t id = task->getFristGetTaskId(); id < task->getNoopTaskId(); ++id) {
        it = mget_running_tasks_.find(id);
        if (it == mget_running_tasks_.end()) {
            continue;
        }
        task = static_cast<MultiGetTask*>(it->second.get());
        task->onResult(id, "", Status(Status::kNotFound, 0));
        mget_running_tasks_.erase(it);//TODO optimize using map::erase(it++)
    }

    task->report();
    mget_running_tasks_.erase(task->id());
    mget_running_tasks_.erase(task->getNoopTaskId());
}

}//end of namespace detail 

}//end of namespace mcx



