#ifndef LISTENER_H
#define LISTENER_H

#include "Attributes.h"
#include "Notification.h"

#include <functional>

namespace EventQueue {

template <typename A, typename T> class EventQueue;

template <typename A, typename T> class Listener {
public:
  using OnNotify = std::function<void(const Notification<T> &)>;

  Listener(const Attributes<A> &attr, OnNotify onNotify)
      : mAttr(attr), mOnNotify(onNotify) {}

  const Attributes<A> &getAttributes() const { return mAttr; }

private:
  friend EventQueue<A, T>;
  void notify(const Notification<T> &notification) { mOnNotify(notification); }

  Attributes<A> mAttr;
  OnNotify mOnNotify;
};
}

#endif // LISTENER_H
