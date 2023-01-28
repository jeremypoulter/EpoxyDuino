#pragma once
#include <map>
#include <atomic>
#include <memory>
#include <mutex>
#include <iostream>
#include <thread>

namespace EpoxyInjection
{

class Event;

class Injector
{
  public:
    static void addEvent(std::unique_ptr<Event> event);
    static size_t eventsSize();

    static void start();
    static void stop();

    // Thread calls
    static void loop();

    static long maxJitter() { return maxJitter_; }

    ~Injector();

  private:
    Injector();

    // microseconds => event
    using Events = std::multimap<long, std::unique_ptr<Event>>;
    static Events events;
    static std::mutex events_mutex;
    static std::atomic<bool> run;
    static std::atomic<bool> do_reset;
    static std::atomic<long> maxJitter_;
    std::thread thr;
};

}
