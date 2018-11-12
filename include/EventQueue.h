#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "ResourceAttr.h"
#include "ResourceHandle.h"
#include "ResourceListener.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template <typename A, typename T> class EventQueue {
public:
  EventQueue() : mRunning(true), mThread([this] { processEntry(); }){};

  ~EventQueue() {
    mRunning = false;
    mEventCv.notify_all();
    mThread.join();
  }

  ResourceHandle<A, T> *provide(const ResourceAttr<A> &attr) {
    auto handle = new ResourceHandle<A, T>(this);

    std::lock_guard<std::mutex> guard(mEventMutex);

    mResourceHandles.emplace(handle, attr);
    return handle;
  }

  void removeProvider(ResourceHandle<A, T> *handle) {
    std::lock_guard<std::mutex> guard(mEventMutex);

    mResourceHandles.erase(handle);
  }

  void listen(ResourceListener<A, T> &listener) {
    std::lock_guard<std::mutex> guard(mListenerMutex);

    mResourceListeners.push_back(&listener);
  }

  void removeListener(ResourceListener<A, T> &listener) {
    std::lock_guard<std::mutex> guard(mListenerMutex);

    mResourceListeners.erase(std::remove(mResourceListeners.begin(),
                                         mResourceListeners.end(), &listener),
                             mResourceListeners.end());
  }

  void waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(mEventMutex);
    mEventCv.wait(lock, [this] { return mQ.empty(); });
  }

private:
  friend ResourceHandle<A, T>;
  void update(ResourceHandle<A, T> *resourceHandle,
              const Notification<T> &notification) {
    std::unique_lock<std::mutex> lock(mEventMutex);

    auto it = mResourceHandles.find(resourceHandle);
    if (it != mResourceHandles.end()) {
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

        for (auto listener : mResourceListeners) {
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
  std::queue<std::pair<ResourceAttr<A>, Notification<T>>> mQ;
  std::map<ResourceHandle<A, T> *, ResourceAttr<A>> mResourceHandles;
  std::vector<ResourceListener<A, T> *> mResourceListeners;
  std::thread mThread;
  std::condition_variable mEventCv;
  std::mutex mEventMutex;
  std::mutex mListenerMutex;
};

#endif // EVENTQUEUE_H
