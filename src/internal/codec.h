#ifndef MCX_MEMCACHED_BINARY_PROTOCOL_CODEC_H
#define MCX_MEMCACHED_BINARY_PROTOCOL_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Memcached.h"

namespace mcx {

class BinaryProtocolCodec : boost::noncopyable
{
public:
    BinaryProtocolCodec();


    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime)
    {
        while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
        {
            // FIXME: use Buffer::peekInt32()
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = muduo::net::sockets::networkToHost32(be32);
            if (len > 65536 || len < 0)
            {
                LOG_ERROR << "Invalid length " << len;
                conn->shutdown();  // FIXME: disable reading
                break;
            }
            else if (buf->readableBytes() >= len + kHeaderLen)
            {
                buf->retrieve(kHeaderLen);
                muduo::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else
            {
                break;
            }
        }
    }

    // FIXME: TcpConnectionPtr
    void send(muduo::net::TcpConnection* conn,
                const muduo::StringPiece& message)
    {
        muduo::net::Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
        buf.prepend(&be32, sizeof be32);
        conn->send(&buf);
    }

public:
    StoreCallback     stored_callback_;
    RemoveCallback    remove_callback_;
    GetCallback       get_callback_;
    MultiGetCallback  mget_callback_;
private:
    StoreCallback     stored_callback_;
    RemoveCallback    remove_callback_;
    GetCallback       get_callback_;
    MultiGetCallback  mget_callback_;

    const static size_t kHeaderLen = sizeof(int32_t);
};

}

#endif  // MCX_MEMCACHED_BINARY_PROTOCOL_CODEC_H
