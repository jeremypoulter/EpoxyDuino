#pragma once
#include <map>
#include <atomic>
#include <memory>
#include <mutex>

#ifdef EPOXY_TEST_DEBUG
#include <iostream>
#define ep_debug(args) { std::cout  << micros() << ' ' << args << std::endl; }
#else
#define ep_debug(args)
#endif

namespace EpoxyInjection
{

class Event
{
  public:
    Event() = default;
    Event(unsigned long time_us);
    virtual ~Event();

    // return value 0 if this is the last event
    virtual unsigned long raise() = 0;
    virtual const char* name() const { return ""; }

    // return value : time to reschedule this
    // or 0 : delete the event
    unsigned long us() { return us_; }

    std::unique_ptr<Event> chain = nullptr;
  private:
    unsigned long us_;  // micros() at which wake up
    static std::atomic<int> count;
};

}
