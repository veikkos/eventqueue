#ifndef NOTIFICATION_H
#define NOTIFICATION_H

namespace EQ {

template <typename T> class Notification {
public:
  Notification(const T &data) : mData(data) {}

  const T getData() const { return mData; }

private:
  T mData;
};
}

#endif // NOTIFICATION_H
