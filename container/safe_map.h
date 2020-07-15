//
// Created by nbaiot@126.com on 2020/6/5.
//

#ifndef NBAIOT_SAFE_MAP_H
#define NBAIOT_SAFE_MAP_H

#include <map>
#include <memory>
#include <functional>
#include <boost/thread/shared_mutex.hpp>

namespace nbaiot {

template<typename TKey, typename TValue, typename Compare = std::less<TKey>>
class SafeMap {
public:
  SafeMap() = default;

  ~SafeMap() {
    Clear();
  }

  bool Insert(const TKey& key, const TValue& value, bool cover = false) {
    WriteLock lk(mutex_);
    auto find = map_.find(key);
    if (find != map_.end() && cover) {
      map_.erase(find);
    }
    auto result = map_.insert(std::pair<TKey, TValue>(key, value));
    return result.second;
  }

  void Remove(const TKey& key) {
    WriteLock lk(mutex_);
    auto find = map_.find(key);
    if (find != map_.end()) {
      map_.erase(find);
    }
  }

  bool Lookup(const TKey& key, TValue& value) {
    ReadLock lk(mutex_);
    auto find = map_.find(key);
    if (find != map_.end()) {
      value = (*find).second;
      return true;
    } else {
      return false;
    }
  }

  int Size() {
    ReadLock lk(mutex_);
    return map_.size();
  }

  bool IsEmpty() {
    return map_.size() == 0;
  }

  void Clear() {
    WriteLock lk(mutex_);
    map_.clear();
  }

  void ForEach(const std::function<bool(const TValue& value)>& func) {
    ReadLock lk(mutex_);
    for(const auto& it: map_) {
      if (!func(it.second))
        break;
    }
  }

  void ForEach(const std::function<bool(const TKey& key, const TValue& value)>& func) {
    ReadLock lk(mutex_);
    for(const auto& it: map_) {
      if (!func(it.first, it.second))
        break;
    }
  }

private:
  typedef boost::shared_lock<boost::shared_mutex> ReadLock;
  typedef boost::unique_lock<boost::shared_mutex> WriteLock;
  boost::shared_mutex mutex_;
  std::map<TKey, TValue, Compare> map_;
};

}

#endif //NBAIOT_SAFE_MAP_H
