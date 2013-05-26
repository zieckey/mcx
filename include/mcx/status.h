#ifndef MCX_STATUS_H__
#define MCX_STATUS_H__

#include <stdio.h>
#include <string>

namespace mcx {

class Status {
  public:
    enum Code {
        kOK = 0,
        kNotFound = 1,
        kTimeout = 2,
        kNetworkError = 3,
        kInvalidArgument = 4,
        kUnknown,
    };

    Status() : code_(kOK), memcached_response_code_(0) { }
    Status(Code ec, int mc_code) 
        : code_(ec), memcached_response_code_(mc_code) { }

    bool operator !() const {
        return code_ != kOK;
    }

    bool ok() const             { return code_ == kOK;          }
    bool isNotFound() const     { return code_ == kNotFound;    }
    bool isTimedOut() const     { return code_ == kTimeout;     }
    bool isNetworkError() const { return code_ == kNetworkError;}
    Code code() const           { return code_;                 }

    void setCode(Code c)        { code_ = c;                    }

    void setMemcachedReponseCode(int c)        
    { memcached_response_code_ = c; }

    std::string toString() const {
        static std::string error_message[] = {
            "OK",
            "NOT FOUND",
            "TIMEOUT",
            "NETWORK ERROR",
            "INVALID ARGUMENT",
            "UNKNOWN ERROR",
        };
        assert(code_ >= 0 && code_ < kUnknown);
        std::string e = error_message[code_];
        char buf[64] = {};
        snprintf(buf, sizeof(buf), " , mccode=%d", 
                    memcached_response_code_);
        e += buf;
        return e;
    }

  private:
    Code code_;
    int  memcached_response_code_;
};

} // namespace mcx

#endif // 
