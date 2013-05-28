#include "task.h"

#include <muduo/base/Logging.h>

#include "memcached_connection.h"
#include "mcx_util.h"

namespace mcx {
namespace detail {

using namespace std;

Task::~Task() {
}

StoreTask::StoreTask(uint32_t task_id, const std::string& k, const std::string& v,
                     uint16_t vbucket_id, const StoreCallback& cb)
    : Task(kStore, task_id), key_(k), value_(v)
    , vbucket_id_(vbucket_id)
    , handler_(cb) 
{
}

void StoreTask::run(MemcachedConnection* m) {
    LOG_TRACE << "store task id=" << id();
    uint32_t expire = 0;
	uint32_t flags  = 0;

    protocol_binary_request_set req;
    memset(&req, 0, sizeof(req));
    req.message.header.request.magic    = PROTOCOL_BINARY_REQ;
    req.message.header.request.opcode   = PROTOCOL_BINARY_CMD_SET;
    req.message.header.request.keylen   = htons(static_cast<uint16_t>(key_.size()));
    req.message.header.request.extlen   = 8;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket  = htons(vbucket_id_);
    req.message.header.request.opaque   = id();
    req.message.header.request.cas      = 0;
    req.message.body.flags              = htonl(flags);
    req.message.body.expiration         = htonl(expire);

    size_t bodylen = req.message.header.request.extlen + 
                        (key_.size()) + 
                        (value_.size());
    req.message.header.request.bodylen  = htonl(static_cast<uint32_t>(bodylen));
    
    TcpConnectionPtr c = m->tcp_client()->connection();
    if (!c) {
        LOG_WARN << m->host() << ":" << m->port() << " NOT CONNECTED!";
        //report(Status(Status::kNetworkError, -1));
        return;
    }

    Buffer buf;
    buf.append(&req, sizeof(req));
    buf.append(key_.data(), key_.size());
    buf.append(value_.data(), value_.size());
    c->send(&buf);
}

void StoreTask::onTimeout()
{
    report(Status(Status::kTimeout, -1));
}

void StoreTask::report(const Status& status)
{
    handler_(key_, status);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
RemoveTask::RemoveTask(uint32_t task_id, const std::string& k, uint16_t vbucket_id,
            const RemoveCallback& cb)
    : Task(kRemove, task_id), key_(k), vbucket_id_(vbucket_id)
    , handler_(cb) {}


void RemoveTask::run(MemcachedConnection* m) {
    LOG_TRACE << "remove task id=" << id();
    
    protocol_binary_request_delete req;
    memset(&req, 0, sizeof(req));
    req.message.header.request.magic    = PROTOCOL_BINARY_REQ;
    req.message.header.request.opcode   = PROTOCOL_BINARY_CMD_DELETE;
    req.message.header.request.keylen   = htons(static_cast<uint16_t>(key_.size()));
    req.message.header.request.extlen   = 0;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket  = htons(vbucket_id_);
    req.message.header.request.bodylen  = htonl(static_cast<uint32_t>(key_.size()));
    req.message.header.request.opaque   = id();
    req.message.header.request.cas      = 0;
    
    TcpConnectionPtr c = m->tcp_client()->connection();
    if (!c) {
        LOG_WARN << m->host() << ":" << m->port() << " NOT CONNECTED!";
        //report(Status(Status::kNetworkError, -1));
        return;
    }

    Buffer buf;
    buf.append(req.bytes, sizeof(req.bytes));
    buf.append(key_.data(), key_.size());
    c->send(&buf);
}

void RemoveTask::onTimeout()
{
    report(Status(Status::kTimeout, -1));
}

void RemoveTask::report(const Status& status)
{
    handler_(key_, status);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
GetTask::GetTask(uint32_t task_id, const std::string& k, uint16_t vbucket_id,
            const GetCallback& cb)
    : Task(kGet, task_id), key_(k), vbucket_id_(vbucket_id)
    , handler_(cb) {}


void GetTask::run(MemcachedConnection* m) {
    LOG_TRACE << "get task id=" << id();
    
    protocol_binary_request_get req;
    memset(&req, 0, sizeof(req));
    req.message.header.request.magic    = PROTOCOL_BINARY_REQ;
    req.message.header.request.opcode   = PROTOCOL_BINARY_CMD_GET;
    req.message.header.request.keylen   = htons(static_cast<uint16_t>(key_.size()));
    req.message.header.request.extlen   = 0;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket  = htons(vbucket_id_);
    req.message.header.request.bodylen  = htonl(static_cast<uint32_t>(key_.size()));
    req.message.header.request.opaque   = id();
    req.message.header.request.cas      = 0;
    
    TcpConnectionPtr c = m->tcp_client()->connection();
    if (!c) {
        LOG_WARN << "task_id=" << id() << " " 
            << m->host() << ":" << m->port() << " NOT CONNECTED!";
        //Status s(Status::kNetworkError, -1);
        //report(GetResult(s,""));
        return;
    }

    Buffer buf;
    buf.append(req.bytes, sizeof(req.bytes));
    buf.append(key_.data(), key_.size());
    c->send(&buf);
}

void GetTask::onTimeout()
{
    report(GetResult(Status(Status::kTimeout, -1), ""));
}

void GetTask::report(const GetResult& r)
{
    handler_(key_, r);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
MultiGetTask::MultiGetTask(uint32_t task_id, 
            const std::vector<KeyEntry>& k,
            const MultiGetCallback& cb)
    : Task(kMultiGet, task_id)
    , first_get_id_(0), noop_cmd_id_(0), keys_(k), handler_(cb) {}


void MultiGetTask::run(MemcachedConnection* m) {
    LOG_TRACE << "multi-get task id=" << id();

    TcpConnectionPtr c = m->tcp_client()->connection();
    if (!c) {
        LOG_WARN << m->host() << ":" << m->port() << " NOT CONNECTED!";
        //Status s(Status::kNetworkError, -1);
        //report(GetResult(s,""));
        return;
    }   

    Buffer buf;
    std::vector<KeyEntry>::const_iterator it (keys_.begin());
    std::vector<KeyEntry>::const_iterator ite(keys_.end());
    for (; it != ite; ++it) {
        const std::string& key = it->key;
        uint16_t vbucket_id    = it->vbucket_id;
        protocol_binary_request_gat req;
        memset(&req, 0, sizeof(req));
        req.message.header.request.magic    = PROTOCOL_BINARY_REQ;
        req.message.header.request.opcode   = PROTOCOL_BINARY_CMD_GETQ;
        req.message.header.request.keylen   = htons(static_cast<uint16_t>(key.size()));
        req.message.header.request.extlen   = 0;
        req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
        req.message.header.request.vbucket  = htons(vbucket_id);
        req.message.header.request.bodylen  = htonl(static_cast<uint32_t>(key.size()));
        req.message.header.request.opaque   = m->nextSeqNo();
        req.message.header.request.cas      = 0;

        buf.append(req.bytes, sizeof(req.bytes) - 4);
        buf.append(key.data(), key.size());
    }

    protocol_binary_request_noop noop;
    memset(&noop, 0, sizeof(noop));
    noop.message.header.request.magic    = PROTOCOL_BINARY_REQ;
    noop.message.header.request.opcode   = PROTOCOL_BINARY_CMD_NOOP;
    noop.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    noop.message.header.request.opaque   = m->nextSeqNo();
    buf.append(noop.bytes, sizeof(noop.bytes));

    noop_cmd_id_  = noop.message.header.request.opaque;
    first_get_id_ = noop_cmd_id_ - static_cast<uint32_t>(keys_.size());//FIXME overflow MAX_UINT32

    LOG_DEBUG << "noop.opaque=" << noop.message.header.request.opaque
        << " noop_cmd_id_=" << noop_cmd_id_ 
        << " first_get_id_=" << first_get_id_ 
        << " task_id=" << id();

    c->send(&buf);
}

void MultiGetTask::onTimeout()
{
    std::vector<KeyEntry>::const_iterator it (keys_.begin());
    std::vector<KeyEntry>::const_iterator ite(keys_.end());
    for (; it != ite; ++it) {
        MultiGetResult::iterator mapiter = results_.find(it->key);
        if (mapiter != results_.end()) {
            continue;
        }
        this->onResult(it->key, "", Status(Status::kTimeout, 0));
    }

    report();
}

void MultiGetTask::onResult(uint32_t cmd_id, const std::string& value, 
            const Status& status)
{
    assert(cmd_id < noop_cmd_id_ && cmd_id >= first_get_id_);
    if (cmd_id >= noop_cmd_id_ || cmd_id < first_get_id_) {
        LOG_ERROR << __func__ << " multi-get task_id=" << id() 
            << "  noop_cmd_id_=" << noop_cmd_id_ 
            << " first_get_id_=" << first_get_id_
            << "    but cmd_id=" << cmd_id;
        return;
    }
    uint32_t index = cmd_id - first_get_id_; 
    assert(index < keys_.size());
    onResult(keys_[index].key, value, status);
}

void MultiGetTask::onResult(const std::string& key, const std::string& value, 
            const Status& status)
{
    GetResult& r = results_[key];
    r.status() = status;
    r.value()  = value;
}

void MultiGetTask::report()
{
    handler_(results_);
}

} // namespace detail 
} // namespace mcx


