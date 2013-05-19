
#ifndef ASMC_MT_RESULT_H_
#define ASMC_MT_RESULT_H_

#include "mt_actions.h"

namespace asmc {

class TaskResult {
public:
    virtual ~TaskResult() {}
    virtual void Report() = 0;
};

class MtPutResult : public TaskResult {
public:
    MtPutResult(StoreTask* action, const Error& error)
        : action_(action), error_(error) {}
    ~MtPutResult() { delete action_; }

    virtual void Report() {
        action_->handler()(action_->key(), error_);
    }
private:
    StoreTask *action_;
    Error error_;
};

class MtDelResult : public TaskResult {
public:
    MtDelResult(RemoveTask* action, const Error& error)
        : action_(action), error_(error) {}
    ~MtDelResult() { delete action_; }

    virtual void Report() {
        action_->handler()(action_->key(), error_);
    }
private:
    RemoveTask *action_;
    Error error_;
};

class MtGetResult : public TaskResult {
public:
    MtGetResult(GetTask* action, const GetResult& result)
        : action_(action), result_(result) {}
    ~MtGetResult() { delete action_; }

    virtual void Report() {
        action_->handler()(action_->key(), result_);
    }
private:
    GetTask *action_;
    GetResult result_;
};

class MtMultiGetResult : public TaskResult {
public:
    MtMultiGetResult(MultiGetTask* action,
                     std::map<std::string, GetResult>* results)
        : action_(action), results_(results) {}
    ~MtMultiGetResult() { delete action_; }

    virtual void Report() {
        action_->handler()(*results_);
    }
private:
    MultiGetTask *action_;
    std::map<std::string, GetResult>* results_;
};

} // namespace asmc


#endif // ASMC_MT_RESULT_H_
