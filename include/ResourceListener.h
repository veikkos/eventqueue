#ifndef RESOURCELISTENER_H
#define RESOURCELISTENER_H

#include "Notification.h"
#include "ResourceAttr.h"

#include <functional>

template <typename A, typename T> class EventQueue;

template <typename A, typename T> class ResourceListener {
public:
  using OnNotify = std::function<void(const Notification<T> &)>;

  ResourceListener(const ResourceAttr<A> &attr, OnNotify onNotify)
      : mAttr(attr), mOnNotify(onNotify) {}

  const ResourceAttr<A> &getAttributes() const { return mAttr; }

private:
  friend EventQueue<A, T>;
  void notify(const Notification<T> &notification) { mOnNotify(notification); }

  ResourceAttr<A> mAttr;
  OnNotify mOnNotify;
};

#endif // RESOURCELISTENER_H
