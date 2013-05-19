#include "task.h"

#include <muduo/base/Logging.h>

#include "memcached_connection.h"
#include "mcx_util.h"

namespace mcx {
namespace detail {

using namespace std;

Task::~Task() {
}

StoreTask::StoreTask(uint32_t _id, const std::string& k, const std::string& v,
                     uint16_t vbucket_id, const StoreCallback& cb)
    : Task(_id), key_(k), value_(v)
    , vbucket_id_(vbucket_id)
    , handler_(cb) 
{
    memset(&req_, 0, sizeof(req_));
}

void StoreTask::run(MemcachedConnection* m) {
    LOG_DEBUG << "Run store task id=" << id();
    uint32_t expire = 0;
	uint32_t flags  = 0;

    req_.message.header.request.magic    = PROTOCOL_BINARY_REQ;
    req_.message.header.request.opcode   = PROTOCOL_BINARY_CMD_SET;
    req_.message.header.request.keylen   = htons(static_cast<uint16_t>(key_.size()));
    req_.message.header.request.extlen   = 8;
    req_.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req_.message.header.request.vbucket  = htons(vbucket_id_);
    req_.message.header.request.opaque   = id();
    req_.message.header.request.cas      = 0;
    req_.message.body.flags              = htonl(flags);
    req_.message.body.expiration         = htonl(expire);

    size_t bodylen = req_.message.header.request.extlen + 
                        (key_.size()) + 
                        (value_.size());
    req_.message.header.request.bodylen  = htonl(static_cast<uint32_t>(bodylen));
    
    TcpConnectionPtr c = m->tcp_client()->connection();
    if (!c) {
        LOG_ERROR << m->host() << ":" << m->port() << " NOT CONNECTED!";
        report(Status(Status::kNetworkError, -1));
        return;
    }

    Buffer buf;
    buf.append(&req_, sizeof(req_));
    buf.append(key_.data(), key_.size());
    buf.append(value_.data(), value_.size());
    c->send(&buf);
}



void StoreTask::report(const Status& status)
{
    handler_(key_, status);
}

//TaskResult* RemoveTask::run(struct memcached_st* m) {
//    uint32_t expire = 0;
//	memcached_return_t ret =
//        memcached_binary_delete(m, key_.data(), key_.size(),
//                                expire, vbucket_id_);
//	if (MEMCACHED_SUCCESS == ret) {
//		return new MtDelResult(this, Error::OK);
//	} else if (MEMCACHED_TIMEOUT == ret) {
//		return new MtDelResult(this, Error::E_TIMED_OUT);
//	} else {
////        const char* szErr = memcached_strerror(m, ret);
////        if (ret == MEMCACHED_ERRNO) {
////            szErr = strerror(errno);
////            fprintf(stderr, "memcached_set errno: (%d) %s\n", errno, szErr);
////        } else {
////            fprintf(stderr, "memcached_set: (%d) %s\n", ret, szErr);
////        }
//		return new MtDelResult(this, Error::E_NOT_FOUND);
//	}
//}
//
//TaskResult* GetTask::run(struct memcached_st* m) {
//	uint32_t flags = 0;
//	size_t valsize = 0;
//	memcached_return_t ret;
//	const char* val =
//        memcached_binary_get(m, key_.data(), key_.size(),
//                             vbucket_id_, &valsize, &flags, &ret);
//    GetResult result;
////    const char* szErr = memcached_strerror(m, ret);
////    if (ret == MEMCACHED_ERRNO) {
////        szErr = strerror(errno);
////        fprintf(stderr, "memcached_set errno: (%d) %s\n", errno, szErr);
////    } else {
////        fprintf(stderr, "memcached_set: (%d) %s\n", ret, szErr);
////    }
//	if (MEMCACHED_SUCCESS == ret) {
//        if (!val) {
//            result.error() = Error::E_NOT_FOUND;
//        } else {
//            result.value().assign(val, valsize);
//            free((void*)val);
//        }
//	} else if (MEMCACHED_NOTFOUND == ret) {
//        result.error() = Error::E_NOT_FOUND;
//	} else if (MEMCACHED_TIMEOUT == ret) {
//        result.error() = Error::E_TIMED_OUT;
//	} else {
//        result.error() = Error::E_NETWORK;
//	}
//	return new MtGetResult(this, result);
//}
//
//struct McResult {
//	memcached_result_st* p;
//	McResult(memcached_result_st* p) : p(p) {}
//	~McResult() { if (p) memcached_result_free(p); }
//};
//
//TaskResult* MultiGetTask::run(struct memcached_st* m) {
//	std::vector<const char*> kptrs(keys_.size());
//	std::vector<size_t>      klens(keys_.size());
//    std::vector<uint16_t>    vbuckets(keys_.size());
//
//    std::vector<KeyEntry>::iterator it = keys_.begin();
//    for (int i = 0; it != keys_.end(); ++it, ++i) {
//        kptrs[i] = it->key.data();
//        klens[i] = it->key.size();
//        vbuckets[i] = it->vbucket;
//	}
//
//	memcached_return_t ret = memcached_binary_mget(m, &kptrs[0],
//            &klens[0], &vbuckets[0], keys_.size());
//    // DEBUG
//    if (ret != MEMCACHED_SUCCESS) {
//        const char* szErr = memcached_strerror(m, ret);
//        if (ret == MEMCACHED_ERRNO) {
//            szErr = strerror(errno);
//            fprintf(stderr, "memcached_mget extern system errno: (%d) %s\n", errno, szErr);
//        } else {
//            fprintf(stderr, "memcached_mget errrno: (%d) %s\n", ret, szErr);
//        }
//    }
//
//    Error::Code ec = Error::OK;
//	if (MEMCACHED_SUCCESS != ret) {
//        ec = Error::E_NETWORK;
//        for (vector<KeyEntry>::iterator it = keys_.begin(); it != keys_.end(); ++it) {
//            results_.insert(make_pair((*it).key, GetResult(ec, "")));
//        }
//        return new MtMultiGetResult(this, &results_);
//	}
//
//    ec = Error::E_NOT_FOUND;
//    for (vector<KeyEntry>::iterator it = keys_.begin(); it != keys_.end(); ++it) {
//        results_.insert(make_pair((*it).key, GetResult(ec, "")));
//    }
//
//    bool successed = false;
//    memcached_result_st* mrt = NULL;
//    while (NULL != (mrt = memcached_fetch_result(m, mrt, &ret))) {
//        // DEBUG
//        if (ret != MEMCACHED_SUCCESS) {
//            const char* szErr = memcached_strerror(m, ret);
//            if (ret == MEMCACHED_ERRNO) {
//                szErr = strerror(errno);
//                fprintf(stderr, "memcached_mget extern system errno: (%d) %s\n", errno, szErr);
//            } else {
//                fprintf(stderr, "memcached_mget errrno: (%d) %s\n", ret, szErr);
//            }
//        }
//
//        if (MEMCACHED_END == ret) {
//            break;
//        }
//
//        std::string key(memcached_result_key_value(mrt), memcached_result_key_length(mrt));
//        if (key.empty()) {
//            continue;
//        }
//
//        std::string value;
//        if (MEMCACHED_SUCCESS == ret) {
//            ec = Error::OK;
//            value.assign(memcached_result_value(mrt), memcached_result_length(mrt));
//            successed = true;
//        } else if (MEMCACHED_NOTFOUND == ret) {
//            ec = Error::E_NOT_FOUND;
//            successed = true;
//        } else if (MEMCACHED_TIMEOUT == ret) {
//            ec = Error::E_TIMED_OUT;
//		} else {
//            ec = Error::E_NETWORK;
//		}
//
//        std::map<std::string, GetResult>::iterator it = results_.find(key);
//        assert(it != results_.end());
//        it->second = GetResult(ec, value);
//	}
//
//    if (NULL != mrt) {
//        memcached_result_free(mrt);
//    }
//
//    if (MEMCACHED_END != ret) {
//        successed = false;
//        if (MEMCACHED_TIMEOUT == ret) {
//            ec = Error::E_TIMED_OUT;
//        } else {
//            ec = Error::E_NETWORK;
//        }
//    }
//    // DEBUG
//    //const char* szErr = memcached_strerror(m, ret);
//    //if (ret == MEMCACHED_ERRNO) {
//    //    szErr = strerror(errno);
//    //    fprintf(stderr, "memcached_mget extern system errno: (%d) %s\n", errno, szErr);
//    //} else {
//    //    fprintf(stderr, "memcached_mget errrno: (%d) %s\n", ret, szErr);
//    //}
//
//    if (!successed) {
//        for (std::map<std::string, GetResult>::iterator it = results_.begin();
//                it != results_.end(); ++it) {
//            it->second = GetResult(ec, "");
//        }
//    }
//
//	return new MtMultiGetResult(this, &results_);
//}

} // namespace detail 
} // namespace mcx


