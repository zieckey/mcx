
#ifndef ASMC_MT_RESULT_H_
#define ASMC_MT_RESULT_H_

#include "tasks.h"

namespace mcx {
namespace detail {

class TaskResult {
public:
    virtual ~TaskResult() {}
    virtual void report() = 0;
};

class StoreTaskResult : public TaskResult {
public:
    StoreTaskResult(StoreTask* task, const Status& status)
        : task_(task), status_(status) {}

    virtual void report() {
        task_->handler()(task_->key(), status_);
    }
private:
    StoreTask *task_;
    Status status_;
};

//class MtDelResult : public TaskResult {
//public:
//    MtDelResult(RemoveTask* task, const Status& status)
//        : task_(task), status_(status) {}
//    ~MtDelResult() { delete task_; }
//
//    virtual void report() {
//        task_->handler()(task_->key(), status_);
//    }
//private:
//    RemoveTask *task_;
//    Status status_;
//};
//
//class MtGetResult : public TaskResult {
//public:
//    MtGetResult(GetTask* task, const GetResult& result)
//        : task_(task), result_(result) {}
//    ~MtGetResult() { delete task_; }
//
//    virtual void report() {
//        task_->handler()(task_->key(), result_);
//    }
//private:
//    GetTask *task_;
//    GetResult result_;
//};
//
//class MtMultiGetResult : public TaskResult {
//public:
//    MtMultiGetResult(MultiGetTask* task,
//                     std::map<std::string, GetResult>* results)
//        : task_(task), results_(results) {}
//    ~MtMultiGetResult() { delete task_; }
//
//    virtual void report() {
//        task_->handler()(*results_);
//    }
//private:
//    MultiGetTask *task_;
//    std::map<std::string, GetResult>* results_;
//};

} // namespace detail
} // namespace mcx 


#endif // ASMC_MT_RESULT_H_
