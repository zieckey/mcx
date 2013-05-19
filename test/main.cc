#include <mcx/memcached.h>
#include <muduo/net/EventLoop.h>
#include <boost/bind.hpp>
#include <stdio.h>

using namespace mcx;

using namespace muduo::net;

EventLoop* g_loop = NULL;

void onGetDone(const std::string& key, const GetResult& result)
{
    printf("Key=[%s] value=[%s] status=[%s]\n", 
                key.data(), result.value().data(), 
                result.status().toString().data());
}

void onStoreDone(const std::string& key, const Status& status)
{
    printf("Key=[%s] status=[%s]\n", 
                key.data(), 
                status.toString().data());
}

void request(Memcached* m)
{
    m->get("abc", &onGetDone);
    m->store("hello", "world", &onStoreDone);
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

    Memcached m("10.16.29.20", 15100);
    m.initialize(&loop);

    loop.runAfter(1, boost::bind(&request, &m));

    loop.loop();
}
