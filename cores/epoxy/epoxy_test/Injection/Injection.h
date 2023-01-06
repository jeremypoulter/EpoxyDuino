#pragma once
#include <map>
#include <atomic>
#include <memory>
#include <mutex>
#include <iostream>

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

class Injector
{
  public:

    Injector();
    ~Injector();

    static void addEvent(std::unique_ptr<Event> event);
    static size_t eventsSize();
    static void reset();

    // Thread calls
    void operator()();

  private:
    // microseconds => event
    using Events = std::multimap<unsigned long, std::unique_ptr<Event>>;
    static Events events;
    static std::mutex events_mutex;
    static Injector injector;
    static std::atomic<bool> run;
    static std::atomic<bool> do_reset;
};

}
