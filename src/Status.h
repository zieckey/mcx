
#ifndef LIBMEMCACHED_MCX_STATUS_H__
#define LIBMEMCACHED_MCX_STATUS_H__

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

    Status() : code_(kOK) { }
    Status(Code ec) : code_(ec) { }

    bool operator !() const {
        return code_ != kOK;
    }

    bool ok() const             { return code_ == kOK;           }
    bool isNotFound() const     { return code_ == kNotFound;    }
    bool isTimedOut() const     { return code_ == kTimeout;     }
    bool isNetworkError() const { return code_ == kNetworkError;}
    Code code() const           { return code_;                 }

    void SetCode(Code c)        { code_ = c;                    }

    std::string toString() const {
        static std::string error_message[] = {
            "ok",
            "not found",
            "time out",
            "network error",
            "invalid argument",
            "unknown error",
        };
        assert(code_ >= 0 && code_ < kUnknown);
        return error_message[code_];
    }

private:
    Code        code_;
};

} // namespace symc

#endif // __SYMC_ERROR_H__
