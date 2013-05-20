#include <mcx/memcached.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <stdio.h>

using namespace mcx;

using namespace muduo::net;

EventLoop* g_loop = NULL;

void onGetDone(const std::string& key, const GetResult& result)
{
    LOG_INFO << "onGetDone:Key=[" << key << "] value=[" << result.value() << "] status=[" << result.status().toString() << "]";
}

void onStoreDone(const std::string& key, const Status& status)
{
    LOG_INFO << "onStoreDone:Key=[" << key << "] status=[" << status.toString() << "]";
}

void onRemoveDone(const std::string& key, const Status& status)
{
    LOG_INFO << "onRemoveDone:Key=[" << key << "] status=[" << status.toString() << "]";
}

void request(Memcached* m)
{
    m->get("abc", &onGetDone);
    m->remove("hello", &onRemoveDone);
    m->store("hello", "world", &onStoreDone);
    m->get("hello", &onGetDone);
    sleep(1);
    m->remove("hello", &onRemoveDone);
    sleep(1);
    m->get("hello", &onGetDone);
}

/*
void done(curl::Request* c, int code)
{
  printf("done %p %s %d\n", c, c->getEffectiveUrl(), code);
}

void done2(curl::Request* c, int code)
{
  printf("done2 %p %s %d %d\n", c, c->getRedirectUrl(), c->getResponseCode(), code);
  // g_loop->quit();
}
*/


int main(int argc, char* argv[])
{
    EventLoop loop;
    g_loop = &loop;
    loop.runAfter(30.0, boost::bind(&EventLoop::quit, &loop));

    Memcached m("localhost", 15100);
    m.initialize(&loop);

    loop.runAfter(1, boost::bind(&request, &m));

    loop.loop();
}
