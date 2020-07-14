//
// Created by cuangenglei-os@360os.com on 2020/5/12.
//
#pragma once

#ifndef NBAIOT_SAFE_UNORDERED_SET_H
#define NBAIOT_SAFE_UNORDERED_SET_H

#include <memory>
#include <functional>
#include <unordered_set>
#include <boost/thread/shared_mutex.hpp>

namespace nbaiot {

template<typename TValue>
class SafeUnorderedSet {
public:
  SafeUnorderedSet() = default;

  ~SafeUnorderedSet() {
    Clear();
  }

  void Insert(TValue value) {
    WriteLock lk(mutex_);
    unordered_set_.insert(value);
  }

  void Remove(TValue value) {
    WriteLock lk(mutex_);
    auto it = unordered_set_.find(value);
    if (it != unordered_set_.end()) {
      unordered_set_.erase(it);
    }
  }

  bool Has(TValue value) {
    ReadLock lk(mutex_);
    auto it = unordered_set_.find(value);
    return it != unordered_set_.end();
  }

  int Size() {
    ReadLock lk(mutex_);
    return unordered_set_.size();
  }

  bool IsEmpty() {
    return unordered_set_.size() == 0;
  }

  void Clear() {
    WriteLock lk(mutex_);
    unordered_set_.clear();
  }

  void ForEach(const std::function<bool (TValue value)>& func) {
    ReadLock lk(mutex_);
    for(const auto& it: unordered_set_) {
      if (!func(it)) {
        break;
      }
    }
  }

private:
  typedef boost::shared_lock<boost::shared_mutex> ReadLock;
  typedef boost::unique_lock<boost::shared_mutex> WriteLock;
  boost::shared_mutex mutex_;
  std::unordered_set<TValue> unordered_set_;
};

}

#endif //NBAIOT_SAFE_UNORDERED_SET_H
