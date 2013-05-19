#ifndef MCX_MEMCACHED_BINARY_CODEC_H
#define MCX_MEMCACHED_BINARY_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <memcached/protocol_binary.h>

namespace mcx {

namespace detail {

class BinaryCodec : boost::noncopyable
{
public:
    BinaryCodec(MemcachedConnection* conn) : conn_(conn) {}


    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime)
    {
        while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 24
        {
            // FIXME: use Buffer::peekInt32()
            const void* data = buf->peek();
            protocol_binary_response_header resp = 
                *static_cast<const protocol_binary_response_header*>(data);
            int bodylen = ntohl(resp.response.bodylen);
            resp.response.bodylen = bodylen;
            resp.response.status  = ntohs(resp.response.status);
            if (bodylen < 0)
            {
                LOG_ERROR << "Invalid length " << bodylen;
                conn->shutdown();  // TODO: reconneted
                break;
            }
            else if (buf->readableBytes() >= bodylen + kHeaderLen)
            {
                onMessage(resp, buf);
            }
            else
            {
                break;
            }
        }
    }

   // // FIXME: TcpConnectionPtr
   // void send(muduo::net::TcpConnection* conn,
   //             const muduo::StringPiece& message)
   // {
   //     muduo::net::Buffer buf;
   //     buf.append(message.data(), message.size());
   //     int32_t len = static_cast<int32_t>(message.size());
   //     int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
   //     buf.prepend(&be32, sizeof be32);
   //     conn->send(&buf);
   // }
private:
    void onMessage(const protocol_binary_response_header& resp, 
                muduo::net::Buffer* buf)
    {
        uint32_t id     = resp.response.opaque;//no need to call ntohl
        int      cmd    = resp.response.opcode;
        switch (cmd) {
            case PROTOCOL_BINARY_CMD_SET:
                conn_->onStoreTaskDone(id, resp.response.status);
            break;
        }
        buf->retrieve(kHeaderLen + resp.response.bodylen);
    }

private:
    MemcachedConnection* conn_;

    static const size_t kHeaderLen = sizeof(protocol_binary_response_header);
};

}
}

#endif  // MCX_MEMCACHED_BINARY_PROTOCOL_CODEC_H
