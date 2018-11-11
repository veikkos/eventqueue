#include "EventQueue.h"
#include "Notification.h"
#include "ResourceAttr.h"
#include "ResourceHandle.h"

#include <iostream>
#include <string>

#include "gtest/gtest.h"

TEST(EventQueue, Example) {
  // Instantiate EventQueue and select resource identification
  // type (here string) and notification type (here unsigned int)
  EventQueue<std::string, unsigned int> q;

  // Create message attributes identifying certain
  // type of resource - use "speed" resource as an
  // example
  const ResourceAttr<std::string> attr("speed");

  // Create a resource which provides "speed"
  ResourceHandle<std::string, unsigned int> *handle = q.provide(attr);

  // Create listener which listens to "speed"
  ResourceListener<std::string, unsigned int> listener(
      attr, [](const Notification<unsigned int> &notification) {
        std::cout << notification.getData() << std::endl;
      });

  // Attach listener to queue
  q.listen(listener);

  // Provider few values for the listener(s)
  handle->update(Notification<unsigned int>(98u));
  handle->update(Notification<unsigned int>(100u));

  // Wait that queue is empty before destroying it
  q.waitUntilEmpty();

  // Retire listener
  q.removeListener(listener);

  // Delete provider. This removes the provider automatically
  // from queue's bookkeeping
  delete handle;
}

TEST(EventQueue, Stress) {
  std::condition_variable cv;
  std::mutex mutex;
  std::atomic_int threadReady(0);
  std::atomic_int threadDone(0);
  std::atomic_bool threadStart(false);
  unsigned int speedNotifications = 0;
  unsigned int altitudeNotifications = 0;
  const unsigned int rounds = 100;

  EventQueue<std::string, unsigned int> q;

  const ResourceAttr<std::string> speedAttr("speed");
  const ResourceAttr<std::string> altitudeAttr("altitude");

  ResourceListener<std::string, unsigned int> speedListener(
      speedAttr,
      [&speedNotifications](const Notification<unsigned int> &notification) {
        ++speedNotifications;
      });

  ResourceListener<std::string, unsigned int> altitudeListener(
      altitudeAttr,
      [&altitudeNotifications](const Notification<unsigned int> &notification) {
        ++altitudeNotifications;
      });

  q.listen(speedListener);
  q.listen(altitudeListener);

  std::thread speedThread([&, rounds] {
    ResourceHandle<std::string, unsigned int> *speedProvider =
        q.provide(speedAttr);

    unsigned int speed = 1u;
    unsigned int r = rounds;

    // Notify test thread that speed thread is ready
    ++threadReady;
    cv.notify_all();

    // Wait flooding permission from test thread
    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&threadStart] { return threadStart.load(); });
    }

    while (r--) {
      speedProvider->update(Notification<unsigned int>(speed++));
    }

    delete speedProvider;

    // Notify test thread that speed thread is done
    ++threadDone;
    cv.notify_all();
  });

  std::thread altitudeThread([&, rounds] {
    ResourceHandle<std::string, unsigned int> *altitudeProvider =
        q.provide(altitudeAttr);

    unsigned int altitude = 2000u;
    unsigned int r = rounds;

    // Notify test thread that altitude thread is ready
    ++threadReady;
    cv.notify_all();

    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&threadStart] { return threadStart.load(); });
    }

    // Wait flooding permission from test thread
    while (r--) {
      altitudeProvider->update(Notification<unsigned int>(altitude++));
    }

    delete altitudeProvider;

    // Notify test thread that altitude thread is done
    ++threadDone;
    cv.notify_all();
  });

  // Wait until threads are ready to start providing
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&threadReady] { return threadReady == 2; });
  }

  // Notify threads that they are free to flood
  threadStart = true;
  cv.notify_all();

  // Wait until threads signal that they are done
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&threadDone] { return threadDone == 2; });
  }

  q.waitUntilEmpty();

  q.removeListener(speedListener);
  q.removeListener(altitudeListener);

  speedThread.join();
  altitudeThread.join();

  EXPECT_EQ(rounds, speedNotifications);
  EXPECT_EQ(rounds, altitudeNotifications);
}
