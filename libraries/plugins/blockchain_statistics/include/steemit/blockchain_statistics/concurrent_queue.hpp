#pragma once
/*
 * concurrent_queue.hpp
 * 
 * This is a simple thread-safe blocking queue, written for safety storing data for stat_sender
 * 
 */
#include <mutex>
#include <condition_variable>
#include <queue>

namespace steemit {
namespace blockchain_statistics {

template < class T, class Container = std::deque<T> >
class concurrent_queue final {
public:
    ~concurrent_queue() {
        synchronize();
    }

    size_t size() const {
        return queue_.size();
    }

    bool empty() const {
        return queue_.empty();
    }

    void synchronize() {
        std::lock_guard<mutex_type> lck(mtx_);
    }

    void clear() {
        std::lock_guard<mutex_type> lck(mtx_);
        while ( !empty() ) {
            queue_.pop();
        }
    }

    void push(const T& x) {
        std::lock_guard<mutex_type> lck(mtx_);
        queue_.push(x);
        if ( size() == 1 ) {
            cv_.notify_one();
        }
    }

    // Wait until non-empty and then pop
    T wait_pop() {
        std::unique_lock<mutex_type> lck(mtx_);
        cv_.wait(lck, [this]() {
                return !empty(); 
            }
        );
        T x = std::move( queue_.front() );
        queue_.pop();
        return std::move( x );
    }
private:
    using mutex_type = std::mutex;
    std::queue<T, Container> queue_;
    mutex_type mtx_;
    std::condition_variable cv_; // notify when the queue becomes non-empty
};


} } // steemit::blockchain_statistics
