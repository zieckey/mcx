#include <mcx/memcached.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <stdio.h>

using namespace mcx;

using namespace muduo::net;

EventLoop* g_loop = NULL;

void onGetDone(const std::string& key, const GetResult& result, int id)
{
    LOG_INFO << "onGetDone: id=" << id << " key=" << key << " value=[" << result.value() << "] status=[" << result.status().toString() << "]";
    if (key == "abc") {
        assert(result.value() == "value-of-abc" || result.status().isTimedOut());
    }

}

void onMultiGetDone(const MultiGetResult& result, int id)
{
    MultiGetResult::const_iterator it (result.begin());
    MultiGetResult::const_iterator ite(result.end());
    for (; it != ite; ++it) {
        LOG_INFO << __func__ << ": id=" << id 
            << " key=" << it->first 
            << " value=[" << it->second.value() << "]" 
            << " status=[" << it->second.status().toString() << "]";
        if (it->first == "abc") {
            assert(it->second.value() == "value-of-abc" || it->second.status().isTimedOut());
        }
    }
}

void onStoreDone(const std::string& key, const Status& status, int id)
{
    LOG_INFO << "onStoreDone: id=" << id << " key=" << key << " status=[" << status.toString() << "]";
}

void onRemoveDone(const std::string& key, const Status& status, int id)
{
    LOG_INFO << "onRemoveDone: id=" << id << " key=" << key << " status=[" << status.toString() << "]";
}

void request(Memcached* m)
{
    int id = 0;
    std::string largevalue(1024*1024, 'c');
    m->store("newkey", largevalue, boost::bind(&onStoreDone, _1, _2, ++id));
    usleep(10000);
    m->get("newkey", boost::bind(&onGetDone, _1, _2, ++id));
    m->store("abc", "value-of-abc", boost::bind(&onStoreDone, _1, _2, ++id));
    usleep(10000);
    m->get("abc", boost::bind(&onGetDone, _1, _2, ++id));
    usleep(10000);
    m->get("abc", boost::bind(&onGetDone, _1, _2, ++id));
    usleep(10000);
    m->remove("hello", boost::bind(&onRemoveDone, _1, _2, ++id));
    usleep(10000);
    m->get("hello", boost::bind(&onGetDone, _1, _2, ++id));
    usleep(10000);
    m->store("hello", "value-of-hello", boost::bind(&onStoreDone, _1, _2, ++id));
    usleep(10000);
    m->get("hello", boost::bind(&onGetDone, _1, _2, ++id));
    usleep(10000);
    m->remove("hello", boost::bind(&onRemoveDone, _1, _2, ++id));
    usleep(10000);
    m->get("hello", boost::bind(&onGetDone, _1, _2, ++id));
    usleep(10000);

    m->store("hello", "value-of-hello", boost::bind(&onStoreDone, _1, _2, ++id));
    m->store("abc1", "value-of-abc1", boost::bind(&onStoreDone, _1, _2, ++id));
    std::vector<std::string> keys;
    keys.push_back("abc");
    keys.push_back("abc");
    keys.push_back("abc1");
    keys.push_back("hello");
    keys.push_back("abc2");
    keys.push_back("hello");
    keys.push_back("hello1");
    keys.push_back("hello2");
    keys.push_back("abc2");
    m->mget(keys, boost::bind(&onMultiGetDone, _1, ++id));
}

int main(int argc, char* argv[])
{
    EventLoop loop;
    g_loop = &loop;
    loop.runAfter(7.0, boost::bind(&EventLoop::quit, &loop));

    Memcached m("10.16.29.21", 11211);
    m.initialize(&loop);
    m.setTimeout(0.1);

    loop.runAfter(1, boost::bind(&request, &m));

    loop.loop();
    LOG_INFO << "exiting ...";
}

