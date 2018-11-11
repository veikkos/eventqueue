#ifndef RESOURCEHANDLE_H
#define RESOURCEHANDLE_H

#include "Notification.h"

template <typename A, typename T> class EventQueue;

template <typename A, typename T> class ResourceHandle {
public:
  ~ResourceHandle() { mQ->removeProvider(this); }

  void update(const Notification<T> &notification) {
    mQ->update(this, notification);
  }

private:
  friend EventQueue<A, T>;
  ResourceHandle(EventQueue<A, T> *q) : mQ(q) {}

  EventQueue<A, T> *mQ;
};

#endif // RESOURCEHANDLE_H
