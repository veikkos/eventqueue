#ifndef RESOURCEATTR_H
#define RESOURCEATTR_H

template <typename T> class ResourceAttr {
public:
  ResourceAttr(const T &resourceId) : mResourceId(resourceId) {}

  const T getResourceId() const { return mResourceId; }

private:
  T mResourceId;
};

#endif // RESOURCEATTR_H
