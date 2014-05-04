#include <mcx/memcached.h>
#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <stdio.h>

using namespace mcx;

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop = NULL;
Memcached* g_mc   = NULL;

struct Stat
{
    AtomicInt32 requesting;
    AtomicInt32 done_ok;
    AtomicInt32 not_found;
    AtomicInt32 done_failed;

    void print()
    {
        LOG_INFO
            << " requesting=" << requesting.get() 
            << " done_ok=" << done_ok.get()
            << " not_found=" << not_found.get()
            << " done_failed=" << done_failed.get();
    }
};

Stat g_stat;

void onGetDone(const std::string& key, const GetResult& result, int id)
{
    LOG_TRACE << "onGetDone: id=" << id << " key=" << key << " value=[" << result.value() << "] status=[" << result.status().toString() << "]";
    //if (key == "abc") {
    //    assert(result.value() == "value-of-abc");
    //}

    if (result.status().ok() || result.status().isNotFound()) {
        g_stat.done_ok.increment();
        if (result.status().isNotFound()) {
            g_stat.not_found.increment();
        }
    } else {
        g_stat.done_failed.increment();
        LOG_ERROR << "onGetDone: id=" << id << " key=" << key << " value=[" << result.value() << "] status=[" << result.status().toString() << "]";
    }
}

void onMultiGetDone(const MultiGetResult& result, int id)
{
    MultiGetResult::const_iterator it (result.begin());
    MultiGetResult::const_iterator ite(result.end());
    for (; it != ite; ++it) {
        LOG_TRACE << __func__ << ": id=" << id 
            << " key=" << it->first 
            << " value=[" << it->second.value() << "]" 
            << " status=[" << it->second.status().toString() << "]";

        if (it->second.status().ok()) {
            assert(it->second.value() == it->first);
        } else {
            LOG_ERROR << "onMultiGetDone: id=" << id << " key=" << it->first << " status=[" << it->second.status().toString() << "]";
        }
    }
}

void onStoreDone(const std::string& key, const Status& status, int id)
{
    LOG_TRACE << "onStoreDone: id=" << id << " key=" << key << " status=[" << status.toString() << "]";

    if (status.ok() || status.isNotFound()) {
        g_stat.done_ok.increment();
    } else {
        LOG_ERROR << "onStoreDone: id=" << id << " key=" << key << " status=[" << status.toString() << "]";
    }
}

void onRemoveDone(const std::string& key, const Status& status, int id)
{
    LOG_INFO << "onRemoveDone: id=" << id << " key=" << key << " status=[" << status.toString() << "]";
}

void request(CountDownLatch* latch)
{
    EventLoop loop;
    g_loop = &loop;
    Memcached m("10.108.72.141", 11511);
    m.initialize(&loop);
    m.setTimeout(1000);
    g_mc = &m;
    latch->countDown();
    loop.loop();
    LOG_INFO << "exiting ...";

    g_mc   = NULL;
    g_loop = NULL;
}

std::string toString(int i) 
{
    char buf[12] = {};
    snprintf(buf, sizeof(buf), "%d", i);
    return buf;
}

int main(int argc, char* argv[])
{
    int batch_count = 5;(void)batch_count;
    CountDownLatch latch(1);
    Thread thread(boost::bind(&request, &latch), "mc-request-th");
    thread.start();
    latch.wait();
    sleep(1);
    g_loop->runEvery(1.0, boost::bind(&Stat::print, &g_stat));
    Memcached* m = g_mc;
    std::vector<std::string> keys;
    for (int i = 0; ; ) {
        char buf[12] = {};
        snprintf(buf, sizeof(buf), "%d", i);
        //m->store(buf, buf, boost::bind(&onStoreDone, _1, _2, i++));
        //i++;
        //keys.clear();
        //    m->store(toString(i), toString(i), boost::bind(&onStoreDone, _1, _2, i));
        //for (int j = 0; j < batch_count; ++j) {
        //    if (i - j >= 0) {
        //        keys.push_back(toString(i-j));
        //    }
        //}
        //m->mget(keys, boost::bind(&onMultiGetDone, _1, i++));
        m->get(toString(i), boost::bind(&onGetDone, _1, _2, i));
        i++;
        g_stat.requesting.increment();
        if (g_stat.requesting.get() > g_stat.done_ok.get() + g_stat.done_failed.get() + 30000) {
            usleep(10);
        }
    }
    LOG_INFO << "do request ...";
    sleep(10);
    LOG_INFO << "exiting ...";
    return 0;
}

