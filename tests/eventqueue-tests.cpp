#include "EventQueue.h"
#include "Notification.h"
#include "ResourceAttr.h"
#include "Resource.h"

#include <iostream>
#include <string>

#include "gtest/gtest.h"

TEST(EventQueue, Example) {
  // Instantiate EventQueue and select resource identification
  // type (here string) and notification type (here unsigned int)
  EventQueue<std::string, unsigned int> q;

  // Create message attributes identifying certain type of
  // resource - use "speed" resource as an example
  const ResourceAttr<std::string> attr("speed");

  // Create a resource which provides "speed"
  auto *resource = q.provide(attr);

  // Create listener which listens to "speed"
  ResourceListener<std::string, unsigned int> listener(
      attr, [](const Notification<unsigned int> &notification) {
        std::cout << notification.getData() << std::endl;
      });

  // Attach listener to queue
  q.listen(listener);

  // Provider few values for the listener(s)
  resource->update(Notification<unsigned int>(98u));
  resource->update(Notification<unsigned int>(100u));

  // Wait until queue is empty before destroying it
  q.waitUntilEmpty();

  // Retire listener
  q.removeListener(listener);

  // Delete provider. This removes the provider automatically
  // from queue's bookkeeping
  delete resource;
}

TEST(EventQueue, Stress) {
  std::condition_variable cv;
  std::mutex mutex;
  std::atomic_int threadReady(0);
  std::atomic_int threadDone(0);
  std::atomic_bool threadStart(false);
  unsigned int speedNotifications = 0;
  unsigned int altitudeNotifications = 0;
  const unsigned int rounds = 1000u;
  const unsigned int initialSpeed = 1u;
  const unsigned int initialAltitude = 2001u;

  EventQueue<std::string, unsigned int> q;

  const ResourceAttr<std::string> speedAttr("speed");
  const ResourceAttr<std::string> altitudeAttr("altitude");

  ResourceListener<std::string, unsigned int> speedListener(
      speedAttr,
      [&speedNotifications](const Notification<unsigned int> &notification) {
        EXPECT_EQ(speedNotifications++ + initialSpeed, notification.getData());
      });

  ResourceListener<std::string, unsigned int> altitudeListener(
      altitudeAttr,
      [&altitudeNotifications](const Notification<unsigned int> &notification) {
        EXPECT_EQ(altitudeNotifications++ + initialAltitude,
                  notification.getData());
      });

  q.listen(speedListener);
  q.listen(altitudeListener);

  std::thread speedThread([&, rounds] {
    auto *speedProvider =
        q.provide(speedAttr);

    unsigned int speed = initialSpeed;
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
    auto *altitudeProvider =
        q.provide(altitudeAttr);

    unsigned int altitude = initialAltitude;
    unsigned int r = rounds;

    ++threadReady;
    cv.notify_all();

    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&threadStart] { return threadStart.load(); });
    }

    while (r--) {
      altitudeProvider->update(Notification<unsigned int>(altitude++));
    }

    delete altitudeProvider;

    ++threadDone;
    cv.notify_all();
  });

  std::thread providerRegisterer([&, rounds] {
    unsigned int r = rounds;

    ++threadReady;
    cv.notify_all();

    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&threadStart] { return threadStart.load(); });
    }

    while (r--) {
      auto *tempProvider =
          q.provide(ResourceAttr<std::string>("providerAttr"));

      tempProvider->update(3u);

      delete tempProvider;
    }

    ++threadDone;
    cv.notify_all();
  });

  std::thread listenerRegisterer([&, rounds] {
    unsigned int r = rounds;

    ++threadReady;
    cv.notify_all();

    {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [&threadStart] { return threadStart.load(); });
    }

    while (r--) {
      ResourceListener<std::string, unsigned int> tempListener(
          ResourceAttr<std::string>("listenerAttr"),
          [](const Notification<unsigned int> &notification) {
            EXPECT_TRUE(false);
          });

      q.listen(tempListener);

      q.removeListener(tempListener);
    }

    ++threadDone;
    cv.notify_all();
  });

  // Wait until threads are ready to start providing
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&threadReady] { return threadReady == 4; });
  }

  // Notify threads that they are free to flood
  threadStart = true;
  cv.notify_all();

  // Wait until threads signal that they are done
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&threadDone] { return threadDone == 4; });
  }

  q.waitUntilEmpty();

  q.removeListener(speedListener);
  q.removeListener(altitudeListener);

  speedThread.join();
  altitudeThread.join();
  providerRegisterer.join();
  listenerRegisterer.join();

  EXPECT_EQ(rounds, speedNotifications);
  EXPECT_EQ(rounds, altitudeNotifications);
}
