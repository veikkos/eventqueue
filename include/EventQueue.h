#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "Attributes.h"
#include "Listener.h"
#include "Resource.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace EQ {

template <typename A, typename T> class EventQueue {
public:
  EventQueue() : mRunning(true), mThread([this] { processEntry(); }){};

  ~EventQueue() {
    mRunning = false;
    mEventCv.notify_all();
    mThread.join();
  }

  Resource<A, T> *provide(const Attributes<A> &attr) {
    auto handle = new Resource<A, T>(this);

    std::lock_guard<std::mutex> guard(mEventMutex);

    mResources.emplace(handle, attr);
    return handle;
  }

  void removeProvider(Resource<A, T> *handle) {
    std::lock_guard<std::mutex> guard(mEventMutex);

    mResources.erase(handle);
  }

  void listen(Listener<A, T> &listener) {
    std::lock_guard<std::mutex> guard(mListenerMutex);

    mListeners.push_back(&listener);
  }

  void removeListener(Listener<A, T> &listener) {
    std::lock_guard<std::mutex> guard(mListenerMutex);

    mListeners.erase(
        std::remove(mListeners.begin(), mListeners.end(), &listener),
        mListeners.end());
  }

  void waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(mEventMutex);
    mEventCv.wait(lock, [this] { return mQ.empty(); });
  }

private:
  friend Resource<A, T>;
  void update(Resource<A, T> *resource, const Notification<T> &notification) {
    std::unique_lock<std::mutex> lock(mEventMutex);

    auto it = mResources.find(resource);
    if (it != mResources.end()) {
      mQ.push(std::make_pair(it->second, notification));
    }

    lock.unlock();
    mEventCv.notify_all();
  }

  void processEntry() {
    while (mRunning) {
      std::unique_lock<std::mutex> lock(mEventMutex);
      mEventCv.wait(lock, [this] { return !mRunning || !mQ.empty(); });

      if (!mRunning)
        break;

      auto event = mQ.front();
      mQ.pop();

      lock.unlock();

      {
        std::lock_guard<std::mutex> guard(mListenerMutex);

        for (auto listener : mListeners) {
          if (event.first.getResourceId() ==
              listener->getAttributes().getResourceId()) {
            listener->notify(event.second);
          }
        }
      }

      mEventCv.notify_all();
    }
  }

  std::atomic_bool mRunning;
  std::queue<std::pair<Attributes<A>, Notification<T>>> mQ;
  std::map<Resource<A, T> *, Attributes<A>> mResources;
  std::vector<Listener<A, T> *> mListeners;
  std::thread mThread;
  std::condition_variable mEventCv;
  std::mutex mEventMutex;
  std::mutex mListenerMutex;
};
}

#endif // EVENTQUEUE_H
