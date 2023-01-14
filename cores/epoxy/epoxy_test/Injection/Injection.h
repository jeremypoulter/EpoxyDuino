#pragma once
#include <map>
#include <atomic>
#include <memory>
#include <mutex>
#include <iostream>

namespace EpoxyInjection
{

class Event;

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
    using Events = std::multimap<long, std::unique_ptr<Event>>;
    static Events events;
    static std::mutex events_mutex;
    static Injector injector;
    static std::atomic<bool> run;
    static std::atomic<bool> do_reset;
};

}
