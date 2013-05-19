#ifndef MCX_MEMCACHED_TASK_H_
#define MCX_MEMCACHED_TASK_H_

#include <vector>
#include <string>

#include <stdint.h>
#include <memcached/protocol_binary.h>

#include "../memcached.h"

namespace mcx {
namespace detail {

class MemcachedConnection;

class Task {
public:
    Task(uint32_t _id) : id_(_id) {}

    uint32_t id() const { return id_; }

    virtual ~Task();
    virtual void run(MemcachedConnection* m) = 0;

    //virtual void report() = 0;

private:
    uint32_t id_;
};

typedef boost::shared_ptr<Task> TaskPtr;


class StoreTask : public Task {
public:
    StoreTask(uint32_t id, const std::string& key, const std::string& value,
              uint16_t vbucket_id, const StoreCallback& handler);

    virtual void run(MemcachedConnection* m);

    void report(const Status& status);

    const std::string&   key() const { return key_; }
    const StoreCallback& handler() const { return handler_; }

private:
    std::string   key_;
    std::string   value_;
    uint16_t      vbucket_id_;
    StoreCallback handler_;

    protocol_binary_request_set req_;
};

//class RemoveTask : public Task {
//public:
//    RemoveTask(const std::string& key, uint16_t vbucket_id,
//                const RemoveCallback& handler)
//        : key_(key), vbucket_id_(vbucket_id)
//        , handler_(handler) {}
//
//    virtual TaskResult* run(struct memcached_st* m);
//
//    const std::string& key() const { return key_; }
//    const RemoveCallback& handler() const { return handler_; }
//private:
//    std::string key_;
//    uint16_t vbucket_id_;
//
//    RemoveCallback handler_;
//};
//
//class GetTask : public Task {
//public:
//    GetTask(const std::string& key, uint16_t vbucket_id,
//            const GetCallback& handler)
//        : key_(key), vbucket_id_(vbucket_id), handler_(handler) {}
//
//    virtual TaskResult* run(struct memcached_st* m);
//
//    const std::string& key() const { return key_; }
//    const GetCallback& handler() const { return handler_; }
//private:
//    std::string key_;
//    uint16_t    vbucket_id_;
//    GetCallback handler_;
//};
//
//class MultiGetTask : public Task {
//public:
//    MultiGetTask(const std::vector<KeyEntry>& keys,
//            const MultiGetCallback& handler)
//		: keys_(keys)
//		, handler_(handler) {}
//
//    virtual TaskResult* run(struct memcached_st* m);
//
//    const std::vector<KeyEntry>& keys() const { return keys_; }
//    const MultiGetCallback& handler() const { return handler_; }
//private:
//    std::vector<KeyEntry> keys_;
//    MultiGetCallback handler_;
//    std::map<std::string, GetResult> results_;
//};

} // namespace detail 
} // namespace mcx 

#endif 
