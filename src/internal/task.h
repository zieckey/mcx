#ifndef MCX_MEMCACHED_TASK_H_
#define MCX_MEMCACHED_TASK_H_

#include <vector>
#include <string>

#include <stdint.h>
#include <memcached/protocol_binary.h>

#include <muduo/net/TimerId.h>

#include "../memcached.h"

namespace mcx {
namespace detail {

class MemcachedConnection;

class Task {
public:
    enum Type {
        kUnkonwn,
        kGet,
        kMultiGet,
        kRemove,
        kStore,
    };

public:
    Task(Type t, uint32_t _id) : type_(t), id_(_id) {}

    void setId(uint32_t k) { id_ = k;   }
    uint32_t id() const { return id_;   }

    Type   type() const { return type_; }

    bool isMultiGet() const { return type_ == kMultiGet; }

    void setTimerId(muduo::net::TimerId tid) { timer_id_ = tid; }
    muduo::net::TimerId getTimerId() const { return timer_id_; }

public:
    virtual ~Task();
    virtual void run(MemcachedConnection* m) = 0;
    virtual void onTimeout() = 0;
    //virtual void report() = 0;


private:
    Type     type_;
    uint32_t id_;
    muduo::net::TimerId timer_id_;
};

class StoreTask : public Task {
public:
    StoreTask(uint32_t id, const std::string& key, const std::string& value,
              uint16_t vbucket_id, const StoreCallback& handler);

    virtual void run(MemcachedConnection* m);
    virtual void onTimeout();

    void report(const Status& status);

    const std::string&   key() const { return key_; }
    const StoreCallback& handler() const { return handler_; }

private:
    std::string   key_;
    std::string   value_;
    uint16_t      vbucket_id_;
    StoreCallback handler_;
};

class RemoveTask : public Task {
public:
    RemoveTask(uint32_t id, const std::string& key, uint16_t vbucket_id,
               const RemoveCallback& handler);

    virtual void run(MemcachedConnection* m);
    virtual void onTimeout();

    void report(const Status& status);

    const std::string& key() const { return key_; }
    const RemoveCallback& handler() const { return handler_; }

private:
    std::string key_;
    uint16_t vbucket_id_;

    RemoveCallback handler_;
};

class GetTask : public Task {
public:
    GetTask(uint32_t id, const std::string& key, uint16_t vbucket_id,
               const GetCallback& handler);

    virtual void run(MemcachedConnection* m);
    virtual void onTimeout();

    void report(const GetResult& status);

    const std::string& key() const { return key_; }
    const GetCallback& handler() const { return handler_; }

private:
    std::string key_;
    uint16_t vbucket_id_;

    GetCallback handler_;
};

struct KeyEntry {
    std::string key;
    uint16_t    vbucket_id;

    KeyEntry() {}
    KeyEntry(const std::string& k, uint16_t id) 
        : key(k), vbucket_id(id) {}
};

class MultiGetTask : public Task {
public:
    MultiGetTask(uint32_t id, 
                 const std::vector<KeyEntry>& keys,
                 const MultiGetCallback& handler);

    //TODO optimize std::vector<KeyEntry>& keys copy problem

    virtual void run(MemcachedConnection* m);
    virtual void onTimeout();

    const std::vector<KeyEntry>& keys() const { return keys_; }
    const MultiGetCallback& handler() const { return handler_; }

    uint32_t getFristGetTaskId() const { return first_get_id_; }
    uint32_t getNoopTaskId()     const { return noop_cmd_id_; }

    void onResult(uint32_t cmd_id, const std::string& value, 
                const Status& status);

    void report();

private:
    void onResult(const std::string& key, const std::string& value, 
                const Status& status);

private:
    uint32_t              first_get_id_;
    uint32_t              noop_cmd_id_;
    std::vector<KeyEntry> keys_;
    MultiGetCallback      handler_;
    MultiGetResult        results_;
};

typedef boost::shared_ptr<Task>         TaskPtr;

} // namespace detail 
} // namespace mcx 

#endif 
