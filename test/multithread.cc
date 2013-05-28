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
    AtomicInt32 done_failed;

    void print()
    {
        LOG_INFO
            << "\n requesting=" << requesting.get() 
            << "\n    done_ok=" << done_ok.get()
            << "\ndone_failed=" << done_failed.get();
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
    } else {
        g_stat.done_failed.increment();
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
            assert(it->second.value() == "value-of-abc");
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

void request(CountDownLatch* latch)
{
    EventLoop loop;
    g_loop = &loop;
    Memcached m("10.16.28.11", 11211);
    m.initialize(&loop);
    m.setTimeout(1000);
    g_mc = &m;
    latch->countDown();
    loop.loop();
    LOG_INFO << "exiting ...";

    g_mc   = NULL;
    g_loop = NULL;
}

int main(int argc, char* argv[])
{
    CountDownLatch latch(1);
    Thread thread(boost::bind(&request, &latch), "request");
    thread.start();
    latch.wait();
    sleep(1);
    g_loop->runEvery(1.0, boost::bind(&Stat::print, &g_stat));
    Memcached* m = g_mc;
    for (int i = 0; ; i++) {
        char buf[12] = {};
        snprintf(buf, sizeof(buf), "%d", i);
        m->get(buf, boost::bind(&onGetDone, _1, _2, i));
        g_stat.requesting.increment();
        if (g_stat.requesting.get() > g_stat.done_ok.get() + g_stat.done_failed.get() + 5000) {
            usleep(10);
        }
    }
    LOG_INFO << "do request ...";
    sleep(10);
    LOG_INFO << "exiting ...";
}

