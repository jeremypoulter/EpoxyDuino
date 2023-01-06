#pragma once
#include <map>
#include <atomic>
#include <memory>
#include <mutex>

#ifndef EPOXY_TEST
#error "You must not include this header, use <epoxy_test/injection> instead."
#endif

namespace EpoxyInjection
{

class Event
{
  public:
    Event(unsigned long time_us);

    // return value 0 if this is the last event
    virtual unsigned long raise() = 0;
    virtual const char* name() const { return ""; }

    // return value : time to reschedule this
    // or 0 : delete the event
    unsigned long us() { return us_; }

    std::unique_ptr<Event> chain = nullptr;
  private:
    unsigned long us_;  // micros() at which wake up
};

}
