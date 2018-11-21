#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

template <typename T> class Attributes {
public:
  Attributes(const T &resourceId) : mResourceId(resourceId) {}

  const T getResourceId() const { return mResourceId; }

private:
  T mResourceId;
};

#endif // ATTRIBUTES_H
