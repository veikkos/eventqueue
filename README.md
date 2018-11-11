# veikkos' C++11 EventQueue

Study project for threaded event queue written in C++11. The goal of the templated header only library is to allow messaging between several threads.

Multiple threads can be used to feed data into EventQueue. EventQueue uses single internal thread for providing the data for listeners.

## Getting started ##

To integrate EventQueue to your own project, you just need to include `include` folder and enable C++11 and threading with your compiler.

Examples can be seen in gtest tests which uses CMake for generating project files.

```
$ mkdir build
$ cd build
$ cmake -G "Unix Makefiles" ..
$ make
$ ./tests/eventqueue-tests
```

## Example usage ##

```
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
```

Above prints

```
98
100
```

See `tests/eventqueue-tests.cpp`.

## Status

[![Build Status](https://api.travis-ci.org/veikkos/eventqueue.svg?branch=public)](https://travis-ci.org/veikkos/eventqueue)
