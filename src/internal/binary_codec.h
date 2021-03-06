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
            const void* data = buf->peek();
            protocol_binary_response_header resp = 
                *static_cast<const protocol_binary_response_header*>(data);
            int bodylen = ntohl(resp.response.bodylen);
            resp.response.bodylen = bodylen;
            resp.response.status  = ntohs(resp.response.status);
            resp.response.keylen  = ntohs(resp.response.keylen);
            if (bodylen < 0)
            {
                LOG_ERROR << "Invalid length " << bodylen;
                conn->shutdown();
                break;
            }
            else if (buf->readableBytes() >= bodylen + kHeaderLen)
            {
                onMessage(resp, buf);
            }
            else
            {
                LOG_TRACE << "need recv more data";
                break;
            }
        }
    }

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
            case PROTOCOL_BINARY_CMD_DELETE:
                conn_->onRemoveTaskDone(id, resp.response.status);
                break;
            case PROTOCOL_BINARY_CMD_GET:
                {
                    const char* pv = buf->peek() + sizeof(resp) + resp.response.extlen;
                    std::string value(pv, resp.response.bodylen - resp.response.extlen);
                    conn_->onGetTaskDone(id, resp.response.status, value);
                }
                break;
            case PROTOCOL_BINARY_CMD_GETQ:
                {
                    const char* pv = buf->peek() + sizeof(resp) + resp.response.extlen;
                    std::string value(pv, resp.response.bodylen - resp.response.extlen);
                    conn_->onMultiGetTaskOneResponse(id, resp.response.status, value);
                    //LOG_DEBUG << "GETQ, opaque=" << id << " value=" << value;
                }
                break;
            case PROTOCOL_BINARY_CMD_NOOP:
                //LOG_DEBUG << "GETQ, NOOP opaque=" << id;
                conn_->onMultiGetTaskDone(id, resp.response.status);
                break;
            default:
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
