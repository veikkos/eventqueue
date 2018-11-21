#ifndef RESOURCE_H
#define RESOURCE_H

#include "Notification.h"

namespace EQ {

template <typename A, typename T> class EventQueue;

template <typename A, typename T> class Resource {
public:
  ~Resource() { mQ->removeProvider(this); }

  void update(const Notification<T> &notification) {
    mQ->update(this, notification);
  }

private:
  friend EventQueue<A, T>;
  Resource(EventQueue<A, T> *q) : mQ(q) {}

  EventQueue<A, T> *mQ;
};
}

#endif // RESOURCE_H
