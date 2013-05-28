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
    LOG_INFO << "MemcachedConnection::~MemcachedConnection[" 
        << host_ << ":" << port_ << "] running_tasks_ empty";
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
    running_tasks_.push(task);
    rotateSeqNo();
}

void MemcachedConnection::cancelTask(const TaskPtr& task)
{
    removeTask(task);
    LOG_TRACE << "cancelTask id=" << task->id();
}

void MemcachedConnection::removeTask(const TaskPtr& task)
{
    TaskPtr runtask = peekTask(task->id());
    running_tasks_.pop();
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
    TaskPtr runtask = peekTask(task_id);
    if (!runtask) {
        return;
    }

    loop_->cancel(runtask->getTimerId());

    assert(dynamic_cast<TaskT*>(runtask.get()));
    TaskT* task = static_cast<TaskT*>(runtask.get());
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

    running_tasks_.pop();
}

void MemcachedConnection::onGetTaskDone(uint32_t task_id, 
            int memcached_response_code, const std::string& return_value)
{
    TaskPtr runtask = peekTask(task_id);
    if (!runtask) {
        return;
    }

    loop_->cancel(runtask->getTimerId());

    assert(dynamic_cast<GetTask*>(runtask.get()));
    assert(runtask->type() == Task::kGet);
    GetTask* task = static_cast<GetTask*>(runtask.get());
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

    running_tasks_.pop();
}

void MemcachedConnection::onMultiGetTaskOneResponse(uint32_t cmd_id, 
            int memcached_response_code, const std::string& return_value)
{
    TaskPtr runtask = peekTask(cmd_id);
    if (!runtask) {
        return;
    }

    loop_->cancel(runtask->getTimerId());

    MultiGetTask* task = static_cast<MultiGetTask*>(runtask.get());
    
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
}

void MemcachedConnection::onMultiGetTaskDone(uint32_t noop_cmd_id, int memcached_response_code)
{
    TaskPtr task = peekTask(noop_cmd_id);
    if (!task) {
        return;
    }

    loop_->cancel(task->getTimerId());

    MultiGetTask* mget_task = static_cast<MultiGetTask*>(task.get());
    assert(mget_task->getNoopTaskId() == noop_cmd_id);

    mget_task->report();
    running_tasks_.pop();
}

TaskPtr MemcachedConnection::peekTask(uint32_t task_id) 
{
    for (TaskPtr task;;)
    {
        task = running_tasks_.front();
        if (task->id() == task_id) {
            return task;
        } else if (task->id() > task_id) {
            LOG_WARN << "error task_id=" << task_id << " Not found in requesting queue";
            return TaskPtr();
        }

        if (task->id() < task_id) {
            if (task->isMultiGet() && task_id <= (static_cast<MultiGetTask*>(task.get()))->getNoopTaskId()) {
                return task;
            }
        }

        task->onTimeout();
        running_tasks_.pop();
    }

    return TaskPtr();
}

}//end of namespace detail 

}//end of namespace mcx



