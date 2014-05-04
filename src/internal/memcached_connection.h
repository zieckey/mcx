#ifndef MEMCACHED_CONNTION_H_
#define MEMCACHED_CONNTION_H_

#include <string>
#include <map>
#include <queue>
#include <muduo/base/Atomic.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>

#include "../memcached.h"
#include "task.h"

namespace mcx
{
namespace detail
{

using namespace muduo;
using namespace muduo::net;

class BinaryCodec;

class MemcachedConnection
{
  public: 
    typedef boost::shared_ptr<TcpClient>   TcpClientPtr;
    typedef boost::shared_ptr<BinaryCodec> BinaryCodecPtr;
    typedef std::map<uint32_t /*task_id*/, TaskPtr> TaskPtrMap;
    typedef std::queue<TaskPtr> TaskPtrQueue;

  public:
    MemcachedConnection(const std::string& srv_host, int listen_port);

    ~MemcachedConnection();

    bool connect(EventLoop* loop);

    /// get next seqno
    uint32_t nextSeqNo() {
        return seqno_.getAndAdd(1);
    }

    void run(TaskPtr& task);
    void cancelTask(const TaskPtr& task);

    TcpClientPtr& tcp_client() {
        return tcp_client_;
    }

    const std::string& host() const { return host_; }
    int port() const { return port_; }


    /// called by Codec
  public:
    void onStoreTaskDone(uint32_t task_id, int memcached_response_code);
    void onRemoveTaskDone(uint32_t task_id, int memcached_response_code);
    void onGetTaskDone(uint32_t task_id, int memcached_response_code, 
                const std::string& return_value);
    void onMultiGetTaskOneResponse(uint32_t task_id, 
                int memcached_response_code, const std::string& return_value);
    void onMultiGetTaskDone(uint32_t noop_cmd_id, int memcached_response_code);

  private:
    template< class TaskT >
    void onTaskDone(uint32_t task_id, int memcached_response_code);
 
    void removeTask(const TaskPtr& task);

    //pop timeout task, and peek the right task (it is also in the queue)
    TaskPtr peekTask(uint32_t task_id);

  private:
    void onConnection(const TcpConnectionPtr& conn);

    void rotateSeqNo();

  private:
    EventLoop*      loop_;
    TcpClientPtr    tcp_client_;
    AtomicInt32     seqno_; //the sequence number

    std::string     host_;
    int             port_;

    TaskPtrQueue    running_tasks_;

    BinaryCodecPtr  codec_;
};
typedef boost::shared_ptr<MemcachedConnection> MemcachedConnectionPtr;

}//namespace mcx
}//namespace detail
#endif

