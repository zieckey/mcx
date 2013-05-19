
#ifndef XASYNC_MEMCACHED_H
#define XASYNC_MEMCACHED_H

#include <string>
#include <map>
#include <vector>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "status.h"

namespace muduo { 
    namespace net { 
        class EventLoop;
    }
}

namespace mcx 
{

class GetResult {
public:
    GetResult() {}

    GetResult(Status e, const std::string& v) 
        : status_(e), value_(v) {}

    const Status& status() const { return status_; }
    const std::string& value() const { return value_; }

    Status& status() {  return status_; }
    std::string& value() { return value_; }

private:
    Status status_;
    std::string value_;
};

typedef std::map<std::string, GetResult> MultiGetResult;

typedef boost::function<
            void(const std::string& key, const Status& status)
                > StoreCallback;
typedef boost::function<
            void(const std::string& key, const Status& status)
                > RemoveCallback;
typedef boost::function<
            void(const std::string& key, const GetResult& result)
                > GetCallback;
typedef boost::function<
            void(const MultiGetResult& results)
                > MultiGetCallback;

class Memcached : public boost::noncopyable
{
public:
    Memcached(const std::string& host, int port);

    ~Memcached();

    bool initialize(muduo::net::EventLoop* loop);

    void store(const std::string& key, const std::string& value,
               const StoreCallback& cb);

    void remove(const std::string& key,
                const RemoveCallback& cb);

    void get(const std::string& key,
             const GetCallback& cb);

    void mget(const std::vector<std::string>& keys,
              const MultiGetCallback& cb);

    //TODO add more interface
private:
    class Impl;
    boost::shared_ptr<Impl> impl_;
};

}

#endif // #ifndef XASYNC_MEMCACHED_H


